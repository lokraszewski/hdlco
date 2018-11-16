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
// #include <serial/serial.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "hdlc/frame.h"
#include "hdlc/hdlc.h"
#include "hdlc/serializer.h"
#include "hdlc/stream_helper.h"

using namespace hdlc;
static auto m_log = spdlog::stdout_color_mt("hdlc");

int run(int argc, char **argv)
{

  m_log->info("Running example.");

  // Default frame
  {
    Frame f;
    m_log->info("Created default frame : {}", f);
  }

  // Info frame
  {
    Frame                f;
    std::vector<uint8_t> payload = {0, 0x7E, 2, 0x7D, 4};
    f.set_address(0xFF);
    f.set_type(Frame::Type::INFORMATION);
    f.set_poll(true);
    f.set_recieve_sequence(1);
    f.set_send_sequence(2);
    f.set_payload(payload);
    m_log->info("Created info frame : {}", f);

    auto f_serial = FrameSerializer::serialize(f);
    m_log->info("Serialized:");
    for (auto c : f_serial) std::cout << fmt::format("{:#x} ", c);
    std::cout << std::endl;

    auto f_escaped = FrameSerializer::escape(f_serial);
    m_log->info("escaped:");
    for (auto c : f_escaped) std::cout << fmt::format("{:#x} ", c);
    std::cout << std::endl;

    // escape
  }

  return 0;
}

int main(int argc, char **argv)
{
  try
  {
    return run(argc, argv);
  }
  catch (std::exception &e)
  {
    m_log->error("Unhandled Exception: {}", e.what());
  }
}
