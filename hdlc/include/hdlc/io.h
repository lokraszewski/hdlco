/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 26-11-2018
 */

#pragma once

#include "frame_pipe.h"
#include "stream_helper.h"
#include "types.h"

namespace hdlc
{
/**
 * @author     lokraszewski
 * @date       21-Nov-2018
 * @brief      Base class for hardware io.
 *
 * @tparam     impl_t  { description }
 *
 * @details    Requires user byte transfer implementaion. Note the pipes are
 *             thread safe.
 */
class base_io
{
public:
  base_io(const size_t buffer_size = 512) : m_out_pipe(buffer_size), m_in_pipe(buffer_size) {}
  virtual ~base_io() {}

  bool send_frame(const Frame& f)
  {
    const auto raw_bytes_tx = FrameSerializer::escape(FrameSerializer::serialize(f));

    if (m_out_pipe.space() < raw_bytes_tx.size())
    {
      // not enough space in the pipe
      return false;
    }

    m_out_pipe.write(raw_bytes_tx);
    return true;
  }

  bool recieve_frame(Frame& f)
  {
    const auto start_tick = get_tick();

    for (;;)
    {
      if (m_in_pipe.frame_count())
      {
        f = FrameSerializer::deserialize(FrameSerializer::descape(m_in_pipe.read_frame()));
        if (f.is_valid())
        {
          return true;
        }
      }
      else if (is_expired(start_tick, m_response_timeout))
      {
        m_in_pipe.clear_partial(); // Clear any partial frames since we dont know if the timeout has occured mid frame.
        return false;
      }
    }
  }

  size_t in_frame_count(void) const { return m_in_pipe.frame_count(); }
  size_t get_elapsed(const size_t tick) const { return get_tick() - tick; }
  bool   is_expired(const size_t tick, const size_t threshold) const { return get_elapsed(tick) > threshold; }

  auto max_send_size() const { return m_out_pipe.capacity(); }
  auto max_recieve_size() const { return m_in_pipe.capacity(); }

  template <typename iter_t>
  auto out_bytes(iter_t begin, iter_t end)
  {
    while (m_out_pipe.empty() == false && begin < end)
    {
      *begin++ = m_out_pipe.read();
    }
    return begin;
  }

  bool out_byte(uint8_t& byte)
  {
    if (m_out_pipe.empty())
      return false;

    byte = m_out_pipe.read();
    return true;
  }

  bool in_byte(const uint8_t byte)
  {
    if (m_in_pipe.full())
      return false;

    m_in_pipe.write(byte);
    return true;
  }

  /*----------  Public interface  ----------*/
  virtual size_t get_tick(void) const   = 0;
  virtual bool   handle_out(void)       = 0;
  virtual bool   handle_in(void)        = 0;
  virtual void   reset()                = 0;
  virtual void   sleep(const size_t ms) = 0;

private:
protected:
  FramePipe    m_out_pipe; //< Contains outgoing data.
  FramePipe    m_in_pipe;  //< Contains incoming data.
  const size_t m_response_timeout = 2000;
};

} // namespace hdlc
