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
#include "hdlc/stream_helper.h"

#include "example_io.h"

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

#if 0
template <typename io_t>
class SessionMaster
{
public:
  SessionMaster(io_t& io, const uint8_t p_address = 0xFF, const uint8_t s_address = 0xFF)
      : m_io(io), m_p_address(p_address), m_s_address(s_address)
  {
  }
  ~SessionMaster() {}

  bool test()
  {
    std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD};
    Frame                f(test_data, Frame::Type::TEST, true, m_s_address); // U frame with test  data.

    std::cout << "Creating frame: " << f << std::endl;
    // Blocking call
    if (m_io.send_frame(f) == false)
    {
      std::cout << "FAILED TO SEND\n";
      return false;
    }

    // Blocking call with timeout.
    if (m_io.recieve_frame(f) == false)
    {
      std::cout << "FAILED TO RECIEVE\n";
      return false;
    }
    else
    {
      if (f.get_payload() == test_data)
      {
        return true;
      }
    }

    return false;
  }

private:
  io_t&   m_io;
  uint8_t m_p_address;
  uint8_t m_s_address;
  uint8_t m_recieve_seq = 0;
  uint8_t m_send_seq    = 0;
};
#endif

int run_normal_master(std::shared_ptr<serial::Serial> port, const uint8_t this_address, const uint8_t target_address)
{

#if 0
  m_log->info("Creating session, this address: {:#x}, target address {:#x}", this_address, target_address);
  io<serial::Serial>                io_serial(port, 512);
  SessionMaster<io<serial::Serial>> session_master(io_serial, this_address, target_address);
#endif

  example_io io(port);
  using namespace std::chrono_literals;
  std::string command = "";

  for (;;)
  {
    m_log->info("Enter command, type 'quit' to exit.");
    std::cin >> command;
    if (command == "quit")
    {
      break;
    }
    else if (command == "rand")
    {
      m_log->info("Testing link...");

      Frame f2;
      auto  f1 = RandomFrameFactory::make();
      if (io.send_frame(f1))
      {
        m_log->info("Sent: {}", f1);
      }

      if (io.recieve_frame(f2))
      {
        m_log->info("Recieved: {}", f2);
      }

      // Frame frame_tx(payload.begin(), payload.end(), Frame::Type::INFORMATION, true, 0xFF, recieve_seq, send_seq);

      // auto success = session_master.test();
      // if (success)
      // {
      // }
      // else
      // {
      //   m_log->error("Link is down.");
      // }
    }
    else
    {
      m_log->error("Unknown command.");
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
  auto              port = std::make_shared<serial::Serial>(port_path, 9600, serial::Timeout::simpleTimeout(500));

  m_log->info("Serial port: {} open: {} ", port_path, port->isOpen());

  if (port->isOpen() == false)
  {
    return -2;
  }

  const auto run_mode = (argc >= 3) ? std::stoi(argv[2]) : 0;
  switch (run_mode)
  {
  case 0: m_log->info("Running listener with echo "); return run_listener(port, true);
  case 1: m_log->info("Running listener without echo "); return run_listener(port, false);
  case 2: m_log->info("Running sender "); return run_sender(port);
  case 3:
  {
    const auto this_address   = (argc >= 4) ? std::stoi(argv[3]) : 1;
    const auto target_address = (argc >= 5) ? std::stoi(argv[4]) : 2;
    return run_normal_master(port, this_address, target_address);
  }
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
