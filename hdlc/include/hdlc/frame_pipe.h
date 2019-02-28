/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 28-02-2019
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
 * @details    Can be used for both sending and recieiving frame buffers. Wraps around boost circular buffer
 */
class FramePipe
{
public:
  FramePipe(const size_t buffer_size) : m_buffer(buffer_size) {}
  ~FramePipe() {}

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Checks if pipe is full.
   *
   * @return     true if full
   *
   */
  auto full() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.full();
  }
  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Checks if pipe is empty
   *
   * @return     True if empty
   */
  auto empty() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.empty();
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns pipe capacity
   *
   * @return     Pipe size is bytes.
   *
   */
  auto capacity() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.capacity();
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns current size
   *
   * @return     Size in bytes
   *
   */
  auto size() const noexcept
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_buffer.size();
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns remaining space
   *
   * @return     Space in bytes
   *
   */
  auto space() const noexcept { return capacity() - size(); }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Clears all bytes.
   */
  void clear(void)
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    m_buffer.clear();
    m_boundary_count = 0;
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns the boundary count.
   *
   * @return     Boundary count
   *
   * @details    The boundary count is the number of frame boundaries received.
   *             This is specici to the HDLC protocol
   */
  auto boundary_count(void) const
  {
#if HDLC_USE_STD_MUTEX
    std::lock_guard<std::mutex> _l(m_mutex);
#endif
    return m_boundary_count;
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns number of potential frames.
   *
   * @return     Boundary count divided by 2 since it takes at least 2
   *             boundaries per frame.
   *
   */
  auto frame_count(void) const { return boundary_count() >> 1; }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Checks if there is a partial frame in the buffer.
   *
   * @return     True if partial frame is in the buffer.
   *
   */
  auto partial_frame(void) const
  {
    const auto cnt = boundary_count();
    return (cnt && cnt & 1) ? true : false;
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Clears any partial frames in the buffer.
   */
  void clear_partial(void)
  {
    // Check if partial frame exsists.
    if (partial_frame())
    {
      // Read until either empty or frame boundary has been found.
      while (!empty() && read() != protocol_bytes::frame_boundary)
        ;
    }
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Reads a single byte
   *
   * @return     0 if empty otherwise next byte.
   *
   * @details    The user should check if the pipe is empty before reading.
   */
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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Reads out all bytes from the pipe
   *
   * @param      buffer  reference to byte buffer
   *
   * @return     Number of bytes read.
   *
   * @details
   */
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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Reads a frame.
   *
   * @return     Vector of bytes which spans a frame or empty vector
   *
   * @details    Finds the boundaries of a frame and extracts it from the
   *             buffer.
   */
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
      // We know we have found 2 boundaries because sof and eof were not end
      m_boundary_count -= 2;
    }

    return buffer;
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Writes a byte into the pipe if not full.
   *
   * @param[in]  byte  The byte
   *
   * @details    Does nothing if the pipe is full. It is the users responsiblity
   *             to check
   */
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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Writes buffer to pipe
   *
   * @param[in]  begin   Begin iterator of the buffer
   * @param[in]  end     End iterator of the buffer
   *
   * @tparam     iter_t  Iterator type
   *
   * @details    Counts boundaries and writes the buffer to the pipe assuming
   *             there is enough space.
   */
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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Same as above
   *
   * @param[in]  buffer  The buffer
   *
   * @details    Same as above except for vector reference.
   */
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
  size_t                          m_boundary_count = 0; //! Counts number of frames in the pipe.
  boost::circular_buffer<uint8_t> m_buffer;             //! Internal storage, relies on boost. It may be better to write or
                                                        // copy the implemenation to drop the reliance on boost.
};

} // namespace hdlc
