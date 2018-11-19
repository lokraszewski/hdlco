/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 19-11-2018
 */

#include "hdlc/serializer.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
namespace hdlc
{

boost::crc_basic<16> FrameSerializer::crc_ccitt = boost::crc_basic<16>(0x1021, 0xFFFF, 0, false, false);

auto FrameSerializer::get_frame_type(const uint8_t control)
{
  if ((control & 1) == 0) // bit 0 clear indicates information frame.
  {
    return Frame::Type::INFORMATION;
  }
  else if ((control & 0b11) == 0b01) // bit pattern 0b01 indicates S frame
  {
    return static_cast<Frame::Type>(control & 0xF);
  }
  else
  {
    auto type = static_cast<Frame::Type>(control & ~(1 << 5));
    switch (type)
    {
    case Frame::Type::INFORMATION:
    case Frame::Type::RECEIVE_READY:
    case Frame::Type::RECEIVE_NOT_READY:
    case Frame::Type::REJECT:
    case Frame::Type::SELECTIVE_REJECT:
    case Frame::Type::UNNUMBERED_INFORMATION:
    case Frame::Type::SET_ASYNCHRONOUS_BALANCED_MODE:
    case Frame::Type::UNNUMBERED_ACKNOWLEDGMENT:
    case Frame::Type::SET_ASYNCHRONOUS_RESPONSE_MODE:
    case Frame::Type::INITIALIZATION:
    case Frame::Type::DISCONNECT:
    case Frame::Type::UNNUMBERED_POLL:
    case Frame::Type::RESET:
    case Frame::Type::EXCHANGE_IDENTIFICATION:
    case Frame::Type::FRAME_REJECT:
    case Frame::Type::NONRESERVED0:
    case Frame::Type::NONRESERVED2:
    case Frame::Type::SET_NORMAL_RESPONSE_MODE:
    case Frame::Type::NONRESERVED1:
    case Frame::Type::NONRESERVED3:
    case Frame::Type::TEST:
    case Frame::Type::UNSET: return type;
    default: return Frame::Type::UNSET;
    }
  }
}

std::vector<uint8_t> FrameSerializer::serialize(const Frame &frame)
{
  std::vector<uint8_t> frame_serialized;
  uint8_t              control_byte = static_cast<uint8_t>(frame.get_type());

  switch (frame.get_type())
  {
  case Frame::Type::RECEIVE_READY:
  case Frame::Type::RECEIVE_NOT_READY:
  case Frame::Type::REJECT:
  case Frame::Type::SELECTIVE_REJECT:
    control_byte |= (frame.get_recieve_sequence() & 0x3) << 5;
    /* fall through*/
  case Frame::Type::INFORMATION: control_byte |= (frame.get_send_sequence() & 0x3) << 1; break;
  case Frame::Type::UNNUMBERED_INFORMATION:
  case Frame::Type::SET_ASYNCHRONOUS_BALANCED_MODE:
  case Frame::Type::UNNUMBERED_ACKNOWLEDGMENT:
  case Frame::Type::SET_ASYNCHRONOUS_RESPONSE_MODE:
  case Frame::Type::INITIALIZATION:
  case Frame::Type::DISCONNECT:
  case Frame::Type::UNNUMBERED_POLL:
  case Frame::Type::RESET:
  case Frame::Type::EXCHANGE_IDENTIFICATION:
  case Frame::Type::FRAME_REJECT:
  case Frame::Type::SET_NORMAL_RESPONSE_MODE:
  case Frame::Type::NONRESERVED0:
  case Frame::Type::NONRESERVED2:
  case Frame::Type::NONRESERVED1:
  case Frame::Type::NONRESERVED3:
  case Frame::Type::TEST: break;
  case Frame::Type::UNSET:
  default: assert(false); // Unknown frame type;
  }

  if (frame.is_poll())
  {
    control_byte |= (1 << 4);
  }

  frame_serialized.emplace_back(protocol_bytes::frame_boundary);
  frame_serialized.emplace_back(frame.get_address());
  frame_serialized.emplace_back(control_byte);

  if (frame.is_payload_type())
  {
    frame_serialized.insert(frame_serialized.end(), frame.begin(), frame.end());
  }

  append_checksum(frame_serialized);
  frame_serialized.emplace_back(protocol_bytes::frame_boundary);

  return frame_serialized;
}
std::vector<uint8_t> FrameSerializer::escape(const std::vector<uint8_t> &frame)
{
  auto extra_size = std::count_if(frame.begin() + 1, frame.end() - 1,
                                  [](const auto byte) { return (byte == protocol_bytes::frame_boundary) || protocol_bytes::escape; });

  std::vector<uint8_t> escaped;
  escaped.reserve(frame.size() + extra_size);
  escaped.emplace_back(protocol_bytes::frame_boundary);

  for_each(frame.begin() + 1, frame.end() - 1, [&](const auto byte) {
    switch (byte)
    {
    case protocol_bytes::frame_boundary:
    case protocol_bytes::escape:
      escaped.emplace_back(protocol_bytes::escape);
      escaped.emplace_back(byte ^ 0x20);
      break;
    default: escaped.emplace_back(byte); break;
    }
  });

  escaped.emplace_back(protocol_bytes::frame_boundary);
  return escaped;
}

Frame FrameSerializer::deserialize(const std::vector<uint8_t> &buffer)
{

  if (buffer.size() < FRAME_MIN_SIZE)
  {
    std::cout << "FRAME TOO SMALL" << std::endl;
    return Frame(Frame::Type::UNSET);
  }

  auto it  = buffer.begin();
  auto end = buffer.end();

  if (!is_checksum_valid(it, end))
  {
    std::cout << "CHECKSUM ERROR" << std::endl;
    return Frame(Frame::Type::UNSET);
  }

  if (*it++ != protocol_bytes::frame_boundary)
  {
    std::cout << "INVALID START" << std::endl;
    return Frame(Frame::Type::UNSET);
  }

  if (*(--end) != protocol_bytes::frame_boundary)
  {
    std::cout << "INVALID END" << std::endl;
    return Frame(Frame::Type::UNSET);
  }

  std::advance(end, -2); // Consume FCS and frame boundary.

  auto       address     = *it++;
  auto       control     = *it++;
  const auto poll        = (control >> 5) & 0b1;
  const auto type        = get_frame_type(control);
  const auto send_seq    = (control >> 1) & 0b111;
  const auto recieve_seq = (control >> 5) & 0b111;

  switch (type)
  {
  case Frame::Type::INFORMATION: return Frame(it, end, type, poll, address, recieve_seq, send_seq);
  case Frame::Type::REJECT:
  case Frame::Type::RECEIVE_READY:
  case Frame::Type::RECEIVE_NOT_READY:
  case Frame::Type::SELECTIVE_REJECT: return Frame(it, end, type, poll, address, recieve_seq);
  case Frame::Type::UNNUMBERED_INFORMATION: return Frame(it, end, type, poll, address);
  case Frame::Type::SET_ASYNCHRONOUS_BALANCED_MODE:
  case Frame::Type::UNNUMBERED_ACKNOWLEDGMENT:
  case Frame::Type::SET_ASYNCHRONOUS_RESPONSE_MODE:
  case Frame::Type::EXCHANGE_IDENTIFICATION:
  case Frame::Type::SET_NORMAL_RESPONSE_MODE:
  case Frame::Type::UNNUMBERED_POLL:
  case Frame::Type::INITIALIZATION:
  case Frame::Type::DISCONNECT:
  case Frame::Type::RESET:
  case Frame::Type::FRAME_REJECT:
  case Frame::Type::NONRESERVED0:
  case Frame::Type::NONRESERVED2:
  case Frame::Type::NONRESERVED1:
  case Frame::Type::NONRESERVED3:
  case Frame::Type::TEST: return Frame(type, poll, address);
  default: std::cout << "UNKNOWN TYPE" << std::endl; return Frame(Frame::Type::UNSET);
  }
}

std::vector<uint8_t> FrameSerializer::descape(const std::vector<uint8_t> &buffer)
{
  const auto escaped = std::count_if(buffer.begin(), buffer.end(), [](const auto byte) { return byte == protocol_bytes::escape; });
  std::vector<uint8_t> descaped;
  descaped.reserve(buffer.size() - escaped);

  for_each(buffer.begin(), buffer.end(), [&](const auto byte) {
    static bool escaped = false;
    if (escaped)
    {
      descaped.emplace_back(byte ^ 0x20);
      escaped = false;
    }
    else if (byte == protocol_bytes::escape)
    {
      escaped = true;
    }
    else
    {
      descaped.emplace_back(byte);
    }
  });

  return descaped;
}

template <typename iterator_t>
auto FrameSerializer::checksum(iterator_t begin, iterator_t end)
{
  crc_ccitt.reset();
  crc_ccitt.process_block(&*begin, &*end);
  return crc_ccitt.checksum();
}

auto FrameSerializer::checksum(std::vector<uint8_t> &frame) { return checksum(frame.begin(), frame.end()); }

void FrameSerializer::append_checksum(std::vector<uint8_t> &buffer)
{
  // Skip over the first byte since we assume it is a frame boundary.
  const auto crc = checksum(buffer.begin() + 1, buffer.end());
  buffer.emplace_back(crc & 0xFF);
  buffer.emplace_back(crc >> 8);
}

template <typename iterator_t>
bool FrameSerializer::is_checksum_valid(iterator_t begin, iterator_t end)
{
  if ((end - begin) < FRAME_MIN_SIZE)
    return false;

  auto expected_begin    = begin + 1;
  auto expected_end      = end - 3;
  auto expected_checksum = checksum(expected_begin, expected_end);
  auto actual_checksum   = *(end - 3) | (*(end - 2) << 8);
  return expected_checksum == actual_checksum;
}

bool FrameSerializer::is_checksum_valid(std::vector<uint8_t> &buffer) { return is_checksum_valid(buffer.begin(), buffer.end()); }

} // namespace hdlc
