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

#include "hdlc/blocking_connection_simple.h"
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
    std::vector<uint8_t> payload = {0, 0x7E, 2, 0x7D, 4};
    Frame                f(payload);
    f.set_poll(true);
    f.set_recieve_sequence(1);
    f.set_send_sequence(2);
    m_log->info("Created info frame : {}", f);

    auto f_serial = FrameSerializer::serialize(f);
    m_log->info("Serialized:");
    for (auto c : f_serial) std::cout << fmt::format("{:#x} ", c);
    std::cout << std::endl;

    auto f_escaped = FrameSerializer::escape(f_serial);
    m_log->info("escaped:");
    for (auto c : f_escaped) std::cout << fmt::format("{:#x} ", c);
    std::cout << std::endl;
  }

  // Mega basic blocking connection.
  {
    m_log->info("Create connection");
    auto connection = std::make_shared<ConnectionBlockingSimple>(
        [](std::vector<uint8_t> &bytes) { return StatusError::ErrorFatal; },
        [](std::vector<uint8_t> &bytes) {
          std::cout << "WRITING: ";
          for (auto c : bytes) std::cout << fmt::format("{:#x} ", c);
          std::cout << std::endl;
          return StatusError::ErrorFatal;
        },
        [](const auto sleep_ms) { std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms)); });

    std::string          test_payload = "TESTING";
    std::vector<uint8_t> test_raw(test_payload.begin(), test_payload.end());
    m_log->info("Sending: |{}|", test_payload);
    connection->write(test_raw);
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
