/*
 * @Author: Lukasz
 * @Date:   19-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 19-11-2018
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

#include "hdlc/hdlc.h"
#include "hdlc/stream_helper.h"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>

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

    REQUIRE(escaped_bytes[6] == 0x7d);
    REQUIRE(escaped_bytes[7] == (0x7e ^ 0x20));

    REQUIRE(escaped_bytes[8] == 0x7d);
    REQUIRE(escaped_bytes[9] == (0x7d ^ 0x20));
  }

  SECTION("De-escape & decode")
  {
    const auto payload = std::vector<uint8_t>({1, 2, 3, 0x7e, 0x7d, 4});
    Frame      frame(payload, Frame::Type::INFORMATION, true, 0x11, 1, 2);

    auto bytes         = FrameSerializer::serialize(frame);
    auto escaped_bytes = FrameSerializer::escape(bytes);

    REQUIRE(bytes == FrameSerializer::descape(escaped_bytes));
    REQUIRE(frame == FrameSerializer::deserialize(FrameSerializer::descape(escaped_bytes)));
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
