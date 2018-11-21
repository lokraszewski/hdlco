#pragma once

#include <stdint.h>

namespace hdlc
{

enum class header_bits : uint8_t
{
  poll_flag = 0x10,
  stuffing = 0x20,
};

enum protocol_bytes : uint8_t
{
  frame_boundary = 0x7E,
  escape         = 0x7D,
};

enum protocol
{
  FRAME_MIN_SIZE = 6,
};

enum class StatusError
{
  Success,
  InvalidParameters,
  ErrorFatal,
  FailedToSend,
  NoResponse,
};

} // namespace hdlc
