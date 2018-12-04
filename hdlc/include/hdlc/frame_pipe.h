/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 26-11-2018
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
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.full();
  }
  auto empty() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.empty();
  }

  auto capacity() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.capacity();
  }

  auto size() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.size();
  }

  auto space() const noexcept { return capacity() - size(); }

  void clear(void)
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    m_buffer.clear();
    m_boundary_count = 0;
  }

  auto boundary_count(void) const
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_boundary_count;
  }
  auto frame_count(void) const { return boundary_count() >> 1; }
  auto partial_frame(void) const
  {
    const auto cnt = boundary_count();
    return cnt && cnt & 1 ? true : false;
  }

  void clear_partial(void)
  {
    if (partial_frame())
    {
      while (!empty() && read() != protocol_bytes::frame_boundary)
        ;
    }
  }

  uint8_t read(void)
  {
    if (empty())
      return 0;
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    const auto byte = m_buffer.front();
    if (byte == protocol_bytes::frame_boundary)
      --m_boundary_count;
    m_buffer.pop_front();

    return byte;
  }

  size_t read(std::vector<uint8_t>& buffer)
  {
    if (empty())
      return 0;

    buffer.reserve(size());

#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif

    std::copy(m_buffer.begin(), m_buffer.end(), std::back_inserter(buffer));
    m_buffer.clear();
    m_boundary_count = 0;
    return buffer.size();
  }

  std::vector<uint8_t> read_frame()
  {
    std::vector<uint8_t> buffer;

    if (frame_count() == 0)
      return buffer;

#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif

    auto begin = m_buffer.begin();
    auto end   = m_buffer.end();
    auto sof   = std::find(begin, end, protocol_bytes::frame_boundary);   // Find start of frame
    auto eof   = std::find(sof + 1, end, protocol_bytes::frame_boundary); // Find end of frame

    // Check if the boundaries are valid.
    if (sof != end && eof++ != end)
    {
      buffer.reserve(eof - sof);
      std::copy(sof, eof, std::back_inserter(buffer));
      m_buffer.erase(sof, eof);
      m_boundary_count -= 2; // We know we have found 2 boundaries because sof and eof were not end
    }

    return buffer;
  }

  void write(const uint8_t byte)
  {
    if (full() == false)
    {
#if HDLC_USE_STD_MUTEX
      std::lock_guard<std::mutex> _l(m_mutex);
#endif
      if (byte == protocol_bytes::frame_boundary)
        ++m_boundary_count;
      m_buffer.push_back(byte);
    }
  }

  template <typename iter_t>
  void write(iter_t begin, iter_t end)
  {
    const size_t requested_size = end - begin;
    if ((requested_size) > space())
      return;

#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif

    m_boundary_count += std::count_if(begin, end, [](const auto byte) { return (byte == protocol_bytes::frame_boundary); });
    m_buffer.insert(m_buffer.end(), begin, end);
  }

  void write(const std::vector<uint8_t>& buffer)
  {
    if (buffer.size() > space())
      return;

#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif

    m_boundary_count +=
        std::count_if(buffer.begin(), buffer.end(), [](const auto byte) { return (byte == protocol_bytes::frame_boundary); });
    m_buffer.insert(m_buffer.end(), buffer.begin(), buffer.end());
  }

private:
#if HDLC_USE_STD_MUTEX
  mutable std::mutex m_mutex;
#endif
  size_t                          m_boundary_count = 0; // Counts number of frames in the pipe.
  boost::circular_buffer<uint8_t> m_buffer;
};

} // namespace hdlc
