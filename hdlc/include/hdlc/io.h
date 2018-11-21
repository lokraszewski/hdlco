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
        if (f.is_empty() == false)
          return true;
      }
      else if (is_expired(start_tick, 1000))
      {
        return false;
      }
    }
  }

  void reset()
  {
    m_out_pipe.clear();
    m_in_pipe.clear();
  }

  size_t in_frame_count(void) const { return m_in_pipe.frame_count(); }

  size_t get_elapsed(const size_t tick) const { return get_tick() - tick; }
  bool   is_expired(const size_t tick, const size_t threshold) const { return get_elapsed(tick) > threshold; }

  /*----------  Public interface  ----------*/
  virtual size_t get_tick(void) const = 0;
  virtual bool   handle_out(void)     = 0;
  virtual bool   handle_in(void)      = 0;

private:
protected:
  FramePipe    m_out_pipe; //< Contains outgoing data.
  FramePipe    m_in_pipe;  //< Contains incoming data.
  const size_t m_response_timeout = 1000;
};

} // namespace hdlc
