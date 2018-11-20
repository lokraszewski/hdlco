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
#include "hdlc/hdlc.h"
#include "hdlc/serializer.h"
#include "hdlc/stream_helper.h"

using namespace hdlc;
static auto m_log = spdlog::stdout_color_mt("hdlc");

std::random_device m_rd;          // Will be used to obtain a seed for the random number engine
std::mt19937       m_gen(m_rd()); // Standard mersenne_twister_engine seeded with rd()

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

  std::uniform_int_distribution<> l_byte(0, 255);
  std::uniform_int_distribution<> l_runs(10, 100);

  m_log->info("Loopback test with {} packets. ", number_of_runs);
  for (auto run = 0; run < number_of_runs; ++run)
  {
    auto packet_len = l_byte(m_gen);
    m_log->info("Packet {} with {} bytes payload. ", run, packet_len);
    std::vector<uint8_t> payload(packet_len);
    for (auto& c : payload) c = l_byte(m_gen);
    Frame frame(payload, Frame::Type::INFORMATION, true, 0xAA, run, run + 1);

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

int run_sender(std::shared_ptr<serial::Serial> port) { return 0; }

int run_listener(std::shared_ptr<serial::Serial> port) { return 0; }

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

  const std::string run_mode = (argc >= 3) ? argv[2] : "loopback";

  m_log->info("Running in {} mode", run_mode);

  if (run_mode == "loopback")
  {
    return run_loopback(port, 10);
  }
  else if (run_mode == "listen")
  {
    return run_listener(port);
  }
  else if (run_mode == "send")
  {
    return run_sender(port);
  }
  else
  {
    m_log->error("Unknown run mode");
    return -1;
  }

  return 0;
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
