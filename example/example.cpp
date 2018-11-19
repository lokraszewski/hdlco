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

namespace
{
};

template <typename T>
void print_bytes(T& bytes)
{
  for (auto c : bytes) std::cout << fmt::format("{:#x} ", c);
  std::cout << std::endl;
}

int run(int argc, char** argv)
{

  m_log->info("Running example.");

  /* TODO. */

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
