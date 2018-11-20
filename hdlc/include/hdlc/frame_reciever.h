/*
 * @Author: Lukasz
 * @Date:   20-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 20-11-2018
 */

#pragma once

#include "types.h"
#include <boost/circular_buffer.hpp>

namespace hdlc
{
class FrameCharReciever
{
public:
  FrameCharReciever(const size_t buffer_size) : m_buffer(buffer_size) {}
  ~FrameCharReciever() {}

  auto frames_in(void) const noexcept { return m_frames_in; }
  auto empty(void) const noexcept { return m_buffer.empty(); }
  auto full(void) const noexcept { return m_buffer.full(); }
  auto begin(void) const { return m_buffer.begin(); }
  auto end(void) const { return m_buffer.end(); }

  auto recieve(const uint8_t byte)
  {

    if (full())
    {
      return false;
    }

    if (m_frame_incoming == false)
    {
      if (byte != protocol_bytes::frame_boundary)
      {
        return false;
      }
      else
      {
        m_frame_incoming = true;
      }
    }
    else
    {
      if (byte == protocol_bytes::frame_boundary)
      {
        m_frame_incoming = false;
        m_frames_in++;
      }
    }

    m_buffer.push_back(byte);

    return true;
  }

  auto recieve(const std::vector<uint8_t> buffer)
  {

    auto bool success = true;
    for (const auto c : buffer)
    {
      success = recieve(c);
    }

    return success;
  }

  auto pop_frame(void)
  {
    std::vector<uint8_t> frame;

    if (m_frames_in == 0)
      return frame;

    auto begin = m_buffer.begin();
    auto end   = m_buffer.end();
    auto sof   = std::find(begin, end, protocol_bytes::frame_boundary);
    auto eof   = std::find(sof + 1, end, protocol_bytes::frame_boundary);

    // Post increment eof since we need the end iterator to point one past the
    // last element for std algorithms.
    if (sof != end && eof++ != end)
    {
      frame.reserve(eof - sof);
      std::copy(sof, eof, std::back_inserter(frame));
      m_buffer.erase(sof, eof);
      --m_frames_in;
    }

    return frame;
  }

  void reset(void)
  {
    m_buffer.clear();
    m_frames_in = 0;
  }

private:
  bool                            m_frame_incoming;
  size_t                          m_frames_in;
  boost::circular_buffer<uint8_t> m_buffer;
};
} // namespace hdlc
