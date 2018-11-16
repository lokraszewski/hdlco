/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 16-11-2018
 */

#pragma once

#include "frame.h"
#include "types.h"

namespace hdlc
{

class FrameFactory
{
public:
  FrameFactory()  = delete;
  ~FrameFactory() = delete;

  Frame make_inforamtion(const uint8_t address, const uint8_t send_seq, const uint8_t recieve_seq)
  {

    Frame f;

    return f;
  }
};
} // namespace hdlc
