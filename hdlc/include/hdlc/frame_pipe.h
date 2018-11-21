/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#pragma once
#include "types.h"
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <vector>

namespace hdlc
{

/**
 * @author     lokraszewski
 * @date       21-Nov-2018
 * @brief      Class for frame pipe.
 *
 * @details    Can be used for both sending and recieiving frame buffers.
 */
class FramePipe
{
public:
  FramePipe(const size_t buffer_size) : m_buffer(buffer_size) {}
  ~FramePipe() {}

  auto full() const noexcept
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_buffer.full();
  }
  auto empty() const noexcept
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_buffer.empty();
  }

  auto space() const noexcept
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_buffer.capacity() - m_buffer.size();
  }

  auto size() const noexcept
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_buffer.size();
  }
  void clear(void)
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    m_buffer.clear();
    m_boundary_count = 0;
  }

  auto frame_count(void) const
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_boundary_count >> 1;
  }
  auto partial_frame(void) const
  {
    std::lock_guard<std::mutex> _l(m_mutex);
    return m_boundary_count & 1 ? true : false;
  }

  void write(const uint8_t byte)
  {
    if (full() == false)
    {
      std::lock_guard<std::mutex> _l(m_mutex);
      if (byte == protocol_bytes::frame_boundary)
        ++m_boundary_count;
      m_buffer.push_back(byte);
    }
  }

  template <typename iter_t>
  void write(iter_t begin, iter_t end)
  {
    if ((end - begin) > space())
      return;
    std::lock_guard<std::mutex> _l(m_mutex);
    m_boundary_count += std::count_if(begin, end, [](const auto byte) { return (byte == protocol_bytes::frame_boundary); });
    m_buffer.insert(m_buffer.end(), begin, end);
  }

  template <typename buffer_t>
  void write(const buffer_t& buffer)
  {
    if (buffer.size() > space())
      return;
    std::lock_guard<std::mutex> _l(m_mutex);
    m_boundary_count +=
        std::count_if(buffer.begin(), buffer.end(), [](const auto byte) { return (byte == protocol_bytes::frame_boundary); });
    m_buffer.insert(m_buffer.end(), buffer.begin(), buffer.end());
  }

private:
  mutable std::mutex              m_mutex;
  size_t                          m_boundary_count = 0; // Counts number of frames in the pipe.
  boost::circular_buffer<uint8_t> m_buffer;
};

} // namespace hdlc
