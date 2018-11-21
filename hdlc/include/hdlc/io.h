/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#pragma once

#include "frame_pipe.h"
#include "types.h"

namespace hdlc
{

template <typename impl_t>
class io
{
public:
  io(std::shared_ptr<impl_t> ptr, const size_t buffer_size) : m_ptr(ptr), m_out_pipe(buffer_size), m_in_pipe(buffer_size) {}
  ~io() {}

  bool send_frame(const Frame& f)
  {
    const auto raw_bytes_tx = FrameSerializer::escape(FrameSerializer::serialize(f));

    std::lock_guard<std::mutex> lock(m_out_mutex);
    const auto                  rem_capacity = m_out_pipe.capacity() - m_out_pipe.size();

    if (rem_capacity < raw_bytes_tx.size())
    {
      return false;
    }

    for (const auto c : raw_bytes_tx)
    {
      m_out_pipe.push_back(c);
    }

    return true;
  }

  bool recieve_frame(Frame& f)
  {

    const auto start_tick = get_tick();

    for (;;)
    {
      if (is_expired(start_tick, 1000))
      {
        std::cout << "TIMEOUT" << std::endl;
        return false;
      }

      if (in_avaiable())
      {
        std::lock_guard<std::mutex> lock(m_in_mutex);
        std::cout << "AVAIABLE" << std::endl;
        const auto begin = m_in_pipe.begin();
        const auto end   = m_in_pipe.end();
        auto       sof   = std::find(begin, end, protocol_bytes::frame_boundary);
        auto       eof   = std::find(sof + 1, end, protocol_bytes::frame_boundary);

        if (sof != end && eof++ != end)
        {
          std::vector<uint8_t> raw_bytes_rx;
          raw_bytes_rx.reserve(eof - sof);
          std::copy(sof, eof, std::back_inserter(raw_bytes_rx));
          m_in_pipe.erase(sof, eof);
          f = FrameSerializer::deserialize(FrameSerializer::descape(raw_bytes_rx));

          std::cout << " Frame in : " << f << std::endl;
          return true;
        }

        // // Post increment eof since we need the end iterator to point one past the
        // // last element for std algorithms.
        // {
        // }
      }
    }

    return false;
  }

  void recieve_callback(const uint8_t byte);

  auto next_out_size(void)
  {
    std::lock_guard<std::mutex> lock(m_out_mutex);
    return m_out_pipe.size();
  }
  auto next_out_avaiable(void)
  {
    std::lock_guard<std::mutex> lock(m_out_mutex);
    return !m_out_pipe.empty();
  }

  auto in_avaiable(void)
  {
    std::lock_guard<std::mutex> lock(m_in_mutex);
    return !m_in_pipe.empty();
  }

  const uint8_t next_out_byte(void) {}

  void handle_out(void)
  {
    if (next_out_avaiable())
    {
      std::lock_guard<std::mutex> lock(m_out_mutex);
      auto                        range1 = m_out_pipe.array_one();
      auto                        range2 = m_out_pipe.array_two();
      m_ptr->write(range1.first, range1.second);
      m_ptr->write(range2.first, range2.second);
      m_out_pipe.clear();
    }
  }
  void handle_in(void)
  {
    if (m_ptr->waitReadable())
    {
      std::lock_guard<std::mutex> lock(m_in_mutex);
      uint8_t                     byte;
      while (m_out_pipe.full() == false && m_ptr->read(&byte, 1))
      {
        m_in_pipe.push_back(byte);
      }
    }
  }

  auto get_tick(void) const
  {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return static_cast<size_t>(now);
  }
  auto get_elapsed(const size_t tick) const { return get_tick() - tick; }
  auto is_expired(const size_t tick, const size_t threshold) const { return get_elapsed(tick) > threshold; }

private:
  FramePipe m_out_pipe;
  FramePipe m_in_pipe;

  std::shared_ptr<impl_t>         m_ptr;
  boost::circular_buffer<uint8_t> m_out_pipe;
  boost::circular_buffer<uint8_t> m_in_pipe;
  std::mutex                      m_out_mutex;
  std::mutex                      m_in_mutex;
  const size_t                    m_response_timeout = 1000;
};

} // namespace hdlc
