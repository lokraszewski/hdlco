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
#include "hdlc/stream_helper.h"

using namespace hdlc;
static auto m_log = spdlog::stdout_color_mt("hdlc");

int run(int argc, char **argv)
{

  m_log->info("Running example.");

  // Default frame
  {
    std::vector<uint8_t> payload = {0, 1, 2, 3, 4};
    Frame                f;
    m_log->info("Created default frame : {}", f);
    f.set_address(0xFF);
    m_log->info(" frame : {}", f);
    f.set_type(Frame::Type::INFORMATION);
    m_log->info(" frame : {}", f);
    f.set_poll(true);
    m_log->info(" frame : {}", f);
    f.set_recieve_sequence(1);
    m_log->info(" frame : {}", f);
    f.set_send_sequence(2);
    m_log->info(" frame : {}", f);
    f.set_payload(payload);
    m_log->info(" frame : {}", f);
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
