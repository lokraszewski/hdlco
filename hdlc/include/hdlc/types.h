#pragma once

#include <stdint.h>

namespace hdlc
{

namespace protocol
{
enum class header_bits : uint8_t
{
  poll_flag = 0x10,
};
}

} // namespace hdlc
