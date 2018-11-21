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

class example_io : public base_io
{

public:
  example_io(std::shared_ptr<serial::Serial> ptr) : base_io(), m_ptr(ptr) {}
  ~example_io() {}

  size_t get_tick(void) const override
  {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return static_cast<size_t>(now);
  }
  bool handle_out(void) override
  {
    bool success = true;
    while (m_out_pipe.empty() == false && success)
    {
      auto byte = m_out_pipe.read();
      success   = (bool)m_ptr->write(&byte, 1);
    }
    return success;
  }
  bool handle_in(void) override
  {
    const auto readable = m_ptr->waitReadable();
    if (readable)
    {
      uint8_t byte;
      while (m_in_pipe.full() == false && m_ptr->read(&byte, 1))
      {
        m_in_pipe.write(byte);
      }
    }
    return readable;
  }

private:
  std::shared_ptr<serial::Serial> m_ptr;
};

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

  // Create a reciever thread. Monitors the serial port and adds characters to the incoming buffer.
  std::thread t_rx([&]() {
    for (;;)
    {
      io.handle_in();
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      if (l_end_of_program == true)
        break;
    }
  });

  // Create the sender thread. sends any outstanding bytes.
  std::thread t_tx([&]() {
    for (;;)
    {
      io.handle_out();
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      if (l_end_of_program == true)
        break;
    }
  }

  );

  for (;;)
  {
    m_log->info("Enter command, type 'quit' to exit.");
    std::cin >> command;
    if (command == "quit")
    {
      std::lock_guard<std::mutex> lock(l_end_of_program_mutex);
      l_end_of_program = true;
      break;
    }
    else if (command == "test")
    {
      m_log->info("Testing link...");

      auto f = RandomFrameFactory::make();
      io.send_frame(f);
      // Frame frame_tx(payload.begin(), payload.end(), Frame::Type::INFORMATION, true, 0xFF, recieve_seq, send_seq);

      // auto success = session_master.test();
      // if (success)
      // {
      //   m_log->info("Link is up.");
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
  case 3:
  {
    const auto this_address   = (argc >= 4) ? std::stoi(argv[3]) : 1;
    const auto target_address = (argc >= 5) ? std::stoi(argv[4]) : 2;
    return run_normal_master(port, this_address, target_address);
  }
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
