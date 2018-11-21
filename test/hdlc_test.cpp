/*
 * @Author: Lukasz
 * @Date:   19-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#include <array>
#include <iostream>
#include <stdint.h>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "hdlc/frame_pipe.h"
#include "hdlc/hdlc.h"
#include "hdlc/random_frame_factory.h"
#include "hdlc/stream_helper.h"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include <catch2/catch.hpp>

static auto l_log = spdlog::stdout_color_mt("hdlc_test");

using namespace hdlc;

TEST_CASE("Frame Creation")
{

  SECTION("Default ctor")
  {
    Frame frame;
    REQUIRE(frame.get_address() == 0xFF);
    REQUIRE(frame.get_type() == Frame::Type::INFORMATION);
    REQUIRE(frame.get_recieve_sequence() == 0);
    REQUIRE(frame.get_send_sequence() == 0);
    REQUIRE(frame.has_payload() == false);
    auto payload = frame.get_payload();
    REQUIRE(payload.size() == 0);
  }

  SECTION("Serializer")
  {
    const auto payload = std::string("PAYLOAD");
    Frame      frame(payload, Frame::Type::INFORMATION, true, 0x11, 1, 2);
    auto       bytes         = FrameSerializer::serialize(frame);
    const auto expected_ctrl = (1 << 5) | (2 << 1) | (1 << 4);

    REQUIRE(frame.is_poll() == true);
    REQUIRE(bytes.size() == (payload.size() + 6));
    REQUIRE(bytes[0] == protocol_bytes::frame_boundary);
    REQUIRE(bytes[1] == 0x11);
    REQUIRE(bytes[2] == expected_ctrl);
    REQUIRE(frame.payload_size() == payload.size());
    REQUIRE(std::equal(frame.begin(), frame.end(), payload.begin()) == true);
    REQUIRE(bytes[12] == protocol_bytes::frame_boundary);
  }

  SECTION("Serializer with escape")
  {
    const auto payload = std::vector<uint8_t>({1, 2, 3, 0x7e, 0x7d, 4});
    Frame      frame(payload, Frame::Type::INFORMATION, true, 0x11, 1, 2);
    auto       bytes         = FrameSerializer::serialize(frame);
    auto       escaped_bytes = FrameSerializer::escape(bytes);

    REQUIRE(frame.get_payload() == payload);
    REQUIRE(escaped_bytes.size() == (bytes.size() + 2));

    REQUIRE(escaped_bytes[3] == 1);
    REQUIRE(escaped_bytes[4] == 2);
    REQUIRE(escaped_bytes[5] == 3);

    REQUIRE(escaped_bytes[6] == 0x7d);
    REQUIRE(escaped_bytes[7] == (0x7e ^ 0x20));

    REQUIRE(escaped_bytes[8] == 0x7d);
    REQUIRE(escaped_bytes[9] == (0x7d ^ 0x20));

    REQUIRE(escaped_bytes[10] == 4);
  }

  SECTION("Compairson operators")
  {
    const auto payload = std::vector<uint8_t>({1, 2, 3, 0x7e, 0x7d, 4});
    Frame      frame1(payload);
    Frame      frame2(payload);
    Frame      frame3;
    REQUIRE(frame1 == frame2);
    REQUIRE(frame2 != frame3);
    REQUIRE(frame1 != frame3);
  }
}

TEST_CASE("Frame Serializer")
{
  SECTION("De-escape & decode")
  {
    // Randomize NUMBER_OF_RUNS frames and make sure when they are
    // serilized/deseriliez that the result matches.
    const auto NUMBER_OF_RUNS = 1000;
    for (auto runs = 0; runs < NUMBER_OF_RUNS; ++runs)
    {
      auto frame         = RandomFrameFactory::make();
      auto bytes         = FrameSerializer::serialize(frame);
      auto escaped_bytes = FrameSerializer::escape(bytes);
      REQUIRE(bytes == FrameSerializer::descape(escaped_bytes));
      REQUIRE(frame == FrameSerializer::deserialize(FrameSerializer::descape(escaped_bytes)));
    }
  }
}

TEST_CASE("Frame Pipe Test")
{
  FramePipe                  pipe1(1024);
  const std::vector<uint8_t> test_data1 = {
      0x7e, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
      22,   23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 0x7e,
  };

  std::vector<uint8_t> test_data2;

  SECTION("Default state.")
  {
    REQUIRE(pipe1.empty() == true);
    REQUIRE(pipe1.size() == 0);
    REQUIRE(pipe1.space() == 1024);
  }

  SECTION("Simple byte operations.")
  {
    pipe1.write(1);
    pipe1.write(2);
    pipe1.write(3);
    pipe1.write(4);

    REQUIRE(pipe1.size() == 4);
    REQUIRE(pipe1.empty() == false);
    REQUIRE(pipe1.frame_count() == 0);
    REQUIRE(pipe1.partial_frame() == false);
    REQUIRE(pipe1.read() == 1);
    REQUIRE(pipe1.read() == 2);
    REQUIRE(pipe1.read() == 3);
    REQUIRE(pipe1.read() == 4);
  }

  SECTION("Byte operations.")
  {
    REQUIRE(pipe1.full() == false);
    for (auto c : test_data1) pipe1.write(c);
    REQUIRE(pipe1.size() == test_data1.size());
    REQUIRE(pipe1.empty() == false);
    REQUIRE(pipe1.frame_count() == 1);
    REQUIRE(pipe1.partial_frame() == false);
    for (const auto c : test_data1) REQUIRE(pipe1.read() == c);
  }

  SECTION("Iterator writes.")
  {
    REQUIRE(pipe1.full() == false);
    pipe1.write(test_data1.begin(), test_data1.end());
    REQUIRE(pipe1.size() == test_data1.size());
    REQUIRE(pipe1.empty() == false);
    REQUIRE(pipe1.frame_count() == 1);
    REQUIRE(pipe1.partial_frame() == false);
  }

  SECTION("Array writes - Read all")
  {
    REQUIRE(pipe1.full() == false);
    pipe1.write(test_data1);
    REQUIRE(pipe1.size() == test_data1.size());
    REQUIRE(pipe1.empty() == false);
    REQUIRE(pipe1.frame_count() == 1);
    REQUIRE(pipe1.partial_frame() == false);

    std::vector<uint8_t> data_out;
    REQUIRE(pipe1.read(data_out) == test_data1.size());
    REQUIRE(data_out == test_data1);
    REQUIRE(pipe1.empty() == true);
    REQUIRE(pipe1.frame_count() == 0);
  }

  SECTION("Array writes - read single frame.")
  {
    pipe1.write(test_data1);
    const auto data_out = pipe1.read_frame();
    REQUIRE(data_out.size() == test_data1.size());
    REQUIRE(data_out == test_data1);
    REQUIRE(pipe1.empty() == true);
    REQUIRE(pipe1.frame_count() == 0);
  }

  SECTION("Array writes - multiple frames.")
  {
    const auto NUMBER_OF_FRAMES = 5;
    for (auto i = NUMBER_OF_FRAMES; i--;) pipe1.write(test_data1);
    REQUIRE(pipe1.frame_count() == NUMBER_OF_FRAMES);

    for (auto i = NUMBER_OF_FRAMES; i--;)
    {
      const auto data_out = pipe1.read_frame();
      REQUIRE(data_out.size() == test_data1.size());
      REQUIRE(data_out == test_data1);
      REQUIRE(pipe1.frame_count() == i);
    }
  }

  // SECTION("Single frames can be recieved.")
  // {
  //   auto frame         = RandomFrameFactory::make_inforamtion(128);
  //   auto escaped_bytes = FrameSerializer::escape(FrameSerializer::serialize(frame));
  //   pipe1.recieve(escaped_bytes); // Pretend to recieve from hardware.
  //   REQUIRE(pipe1.frames_in() == 1);
  //   auto rxbytes = pipe1.pop_frame();
  //   REQUIRE(pipe1.empty() == true);
  //   REQUIRE(rxbytes.size() == escaped_bytes.size());
  //   REQUIRE(frame == FrameSerializer::deserialize(FrameSerializer::descape(rxbytes)));
  // }
  // SECTION("Mulitple frames can be received")
  // {
  //   const auto NUMBER_OF_FRAMES = 3;
  //   auto       f1               = RandomFrameFactory::make_inforamtion(128);
  //   auto       f2               = RandomFrameFactory::make_inforamtion(128);
  //   auto       f3               = RandomFrameFactory::make_inforamtion(128);
  //   pipe1.recieve(FrameSerializer::escape(FrameSerializer::serialize(f1))); // Pretend to recieve from hardware.
  //   pipe1.recieve(FrameSerializer::escape(FrameSerializer::serialize(f2))); // Pretend to recieve from hardware.
  //   pipe1.recieve(FrameSerializer::escape(FrameSerializer::serialize(f3))); // Pretend to recieve from hardware.
  //   REQUIRE(f1 == FrameSerializer::deserialize(FrameSerializer::descape(pipe1.pop_frame())));
  //   REQUIRE(f2 == FrameSerializer::deserialize(FrameSerializer::descape(pipe1.pop_frame())));
  //   REQUIRE(f3 == FrameSerializer::deserialize(FrameSerializer::descape(pipe1.pop_frame())));
  // }
}
