/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 19-11-2018
 */

#pragma once

#include "frame.h"
#include "types.h"
#include <boost/crc.hpp>

namespace hdlc
{

class FrameSerializer
{
public:
  static std::vector<uint8_t> serialize(const Frame &frame);
  static std::vector<uint8_t> escape(const std::vector<uint8_t> &frame);

  static Frame                deserialize(const std::vector<uint8_t> &buffer);
  static std::vector<uint8_t> descape(const std::vector<uint8_t> &buffer);

  template <typename iterator_t>
  static auto checksum(iterator_t begin, iterator_t end);
  static auto checksum(std::vector<uint8_t> &frame);
  static void append_checksum(std::vector<uint8_t> &buffer);
  template <typename iterator_t>
  static bool is_checksum_valid(iterator_t begin, iterator_t end);
  static bool is_checksum_valid(std::vector<uint8_t> &buffer);

private:
  static auto                 get_frame_type(const uint8_t control);
  static boost::crc_basic<16> crc_ccitt;
};

} // namespace hdlc
