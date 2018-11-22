#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable> // std::condition_variable
#include <cstdio>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <random>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <serial/serial.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "hdlc/frame.h"
#include "hdlc/frame_reciever.h"
#include "hdlc/hdlc.h"
#include "hdlc/io.h"
#include "hdlc/random_frame_factory.h"
#include "hdlc/serializer.h"
#include "hdlc/session_master.h"
#include "hdlc/stream_helper.h"

#include "example_io.h" //IO implementation for HDLC.

using namespace hdlc;
static auto m_log = spdlog::stdout_color_mt("hdlc");

template <typename T>
void print_bytes(T& bytes)
{
  for (auto c : bytes) std::cout << fmt::format("{:#x} ", c);
  std::cout << std::endl;
}

int run_sender(std::shared_ptr<serial::Serial> port)
{
  example_io  io(port); // create io port.
  std::string payload = "";
  while (payload != "quit")
  {
    m_log->info("Please enter command ('quit' to stop)");
    std::cin >> payload;
    Frame f(payload.begin(), payload.end());
    if (io.send_frame(f))
    {
      m_log->info("Sent : {}", f);
    }
    else
    {
      m_log->error("Failed to send : {}", f);
    }
  }

  return 0;
}

int run_listener(std::shared_ptr<serial::Serial> port, bool echo)
{
  example_io io(port); // create io port.

  // auto io = std::make_shared
  for (;;)
  {
    if (io.in_frame_count())
    {
      Frame f;
      if (io.recieve_frame(f))
      {
        if (echo)
          io.send_frame(f);

        m_log->info("Recieved: {}", f);
        if (f.has_payload())
        {
          std::string payload(f.begin(), f.end());
          m_log->info("Payload: {}", payload);
          if ("quit" == payload)
            return 0;
        }
      }
    }
  }
}

int run_normal_master(std::shared_ptr<serial::Serial> port, const uint8_t this_address, const uint8_t target_address)
{

  example_io                io(port);
  SessionMaster<example_io> session(io, this_address, target_address);

  using namespace std::chrono_literals;

  for (;;)
  {
    if (session.connected())
    {
      const auto err = session.test();
      if (err != StatusError::Success)
      {
        m_log->error("Connection test failed: {}", err);
      }
      // std::this_thread::sleep_for(2s);
    }
    else
    {
      m_log->info("Not connected attempting to connect.");
      const auto err = session.connect();
      if (err == StatusError::Success)
      {
        m_log->info("Successfully connected.");
      }
      else
      {
        m_log->error("Failed to connect : {}", err);
      }
    }
  }

  return 0;
}

int run_echo_client(std::shared_ptr<serial::Serial> port, const uint8_t this_address, const uint8_t target_address)
{

  using namespace std::chrono_literals;
  example_io                io(port);
  SessionClient<example_io> session(io, this_address, target_address);
  ConnectionStatus          status = ConnectionStatus::Disconnected;

  for (;;)
  {
    const auto new_status = session.run();
    if (new_status != status)
    {
      m_log->info("New status: {}", new_status);
      status = new_status;
    }
  }

  return 0;
}

int run(int argc, char** argv)
{

  if (argc < 2)
  {
    m_log->error("Incorrect parameters! ");
    return -1;
  }

  const std::string port_path(argv[1]);
  auto              port = std::make_shared<serial::Serial>(port_path, 9600, serial::Timeout::simpleTimeout(10));

  m_log->info("Serial port: {} open: {} ", port_path, port->isOpen());

  if (port->isOpen() == false)
  {
    return -2;
  }

  const auto run_mode       = (argc >= 3) ? std::stoi(argv[2]) : 0;
  const auto this_address   = (argc >= 4) ? std::stoi(argv[3]) : 0xFF;
  const auto target_address = (argc >= 5) ? std::stoi(argv[4]) : 0xFF;

  switch (run_mode)
  {
  case 0: m_log->info("Running listener with echo "); return run_listener(port, true);
  case 1: m_log->info("Running listener without echo "); return run_listener(port, false);
  case 2: m_log->info("Running sender "); return run_sender(port);
  case 3:
    m_log->info("Running master session, this address : {}, secondary address {}", this_address, target_address);
    return run_normal_master(port, this_address, target_address);
  case 4:
    m_log->info("Running echo client, this address : {}, secondary address {}", this_address, target_address);
    return run_echo_client(port, this_address, target_address);
  default: m_log->error("Unknown mode! "); return -1;
  }
}

int main(int argc, char** argv)
{
  try
  {
    return run(argc, argv);
  }
  catch (std::exception& e)
  {
    m_log->error("Unhandled Exception: {}", e.what());
  }
}
