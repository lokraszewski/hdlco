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

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <serial/serial.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "hdlc/frame.h"
#include "hdlc/frame_reciever.h"
#include "hdlc/hdlc.h"
#include "hdlc/random_frame_factory.h"
#include "hdlc/serializer.h"
#include "hdlc/stream_helper.h"

using namespace hdlc;
static auto m_log = spdlog::stdout_color_mt("hdlc");

std::mutex l_end_of_program_mutex;
bool       l_end_of_program = false;

namespace
{
};

template <typename T>
void print_bytes(T& bytes)
{
  for (auto c : bytes) std::cout << fmt::format("{:#x} ", c);
  std::cout << std::endl;
}

int run_loopback(std::shared_ptr<serial::Serial> port, const int number_of_runs)
{
  port->flush(); // flush both input and output.

  m_log->info("Loopback test with {} packets. ", number_of_runs);
  for (auto run = 0; run < number_of_runs; ++run)
  {
    auto frame = RandomFrameFactory::make_inforamtion(128);
    m_log->info("Sending frame: {}", frame);
    const auto raw_bytes_tx = FrameSerializer::escape(FrameSerializer::serialize(frame));
    port->write(raw_bytes_tx);

    std::vector<uint8_t> bytes_raw;
    if (port->read(bytes_raw, raw_bytes_tx.size()) == raw_bytes_tx.size())
    {
      auto frame_rx = FrameSerializer::deserialize(FrameSerializer::descape(bytes_raw));
      if (frame_rx != frame)
      {
        m_log->error(" Loopback failed, \n\tsend {}, \n\trecieved: {} \n\rRAW:", frame, frame_rx);
        print_bytes(bytes_raw);
        return -1;
      }
    }
    else
    {
      m_log->error("Loopback failed for frame: {} ", frame);
      return -1;
    }
  }

  return 0;
}

int run_sender(std::shared_ptr<serial::Serial> port)
{

  port->flush(); // flush both input and output.

  std::string payload     = "";
  uint8_t     recieve_seq = 0;
  uint8_t     send_seq    = 0;
  while (payload != "quit")
  {
    m_log->info("Please enter command ('quit' to stop)");
    std::cin >> payload;

    Frame frame_tx(payload.begin(), payload.end(), Frame::Type::INFORMATION, true, 0xFF, recieve_seq, send_seq);

    m_log->info("Sending : {}", frame_tx);
    port->write(FrameSerializer::escape(FrameSerializer::serialize(frame_tx)));
  }

  return 0;
}

int run_listener(std::shared_ptr<serial::Serial> port)
{

  port->flush(); // flush both input and output.
  FrameCharReciever rx(512);
  uint8_t           byte;

  for (;;)
  {
    while (!rx.full() && port->read(&byte, 1))
    {
      rx.recieve(byte);
    }

    if (rx.frames_in())
    {
      auto frame_rx = FrameSerializer::deserialize(FrameSerializer::descape(rx.pop_frame()));
      m_log->info("Recieved : {}", frame_rx);

      if (frame_rx.has_payload())
      {
        std::string payload(frame_rx.begin(), frame_rx.end());
        m_log->info("Payload: {}", payload);
        if ("quit" == payload)
          return 0;
      }
    }
  }

  return 0;
}

int run_normal_master(std::shared_ptr<serial::Serial> port)
{
#if 0
  hdlc::io io_serial(port);

  m_log->info("Creating session to slave at address {:#x}", 0xFF);

  hdlc::SessionMaster session_master(io_serial, 1, 2);
  hdlc::SessionSlave session_slave(io_serial, 2, 1);

  session_master.is_connected()
  session_master.run(); //Normally on its own thread.
  session_master.connect()
  session_slave.is_connected()

#endif
  using namespace std::chrono_literals;

  std::string command = "";

  // Create a reciever thread. Monitors the serial port and adds characters to the incoming buffer.
  std::thread t_rx([&]() {
    for (;;)
    {
      std::this_thread::sleep_for(1s);
      std::cout << "RX\n";
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      if (l_end_of_program == true)
        break;
    }
  }

  );

  // Create the sender thread. sends any outstanding bytes.
  std::thread t_tx([&]() {
    for (;;)
    {
      std::this_thread::sleep_for(1s);
      std::cout << "TX\n";
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      if (l_end_of_program == true)
        break;
    }
  }

  );

  for (;;)
  {
    std::cin >> command;
    if (command == "quit")
    {
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      l_end_of_program = true;
      break;
    }
  }

  t_rx.join();
  t_tx.join();

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

  m_log->info("Running in {} mode", run_mode);

  switch (run_mode)
  {
  case 1: return run_sender(port);
  case 2: return run_listener(port);
  case 3: return run_normal_master(port);
  default: return run_loopback(port, 10);
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
