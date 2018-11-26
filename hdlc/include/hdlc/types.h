#pragma once

#include <stdint.h>

#ifdef __unix__
#define HDLC_USE_IO_STREAM 1
#else
#define HDLC_USE_IO_STREAM 0
#endif

namespace hdlc
{

enum class header_bits : uint8_t
{
  poll_flag = 0x10,
  stuffing  = 0x20,
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
  InvalidResponse,
  InvalidSequence,
  InvalidAddress,
  InvalidRequest,
  ConnectionError,
  FailedToSend,
  NoResponse,
  Busy,
};

enum class ConnectionStatus
{
  Disconnected,
  Connecting,
  Connected,
};

} // namespace hdlc
