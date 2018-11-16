#pragma once

#include <stdint.h>

namespace hdlc
{

enum class header_bits : uint8_t
{
  poll_flag = 0x10,
};

enum protocol_bytes : uint8_t
{
  frame_boundary = 0x7E,
  escape         = 0x7D,
};

} // namespace hdlc
