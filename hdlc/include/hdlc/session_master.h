/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#pragma once

#include "frame.h"
#include "io.h"
#include "types.h"

#include <vector>

namespace hdlc
{

template <typename io_t>
class Session
{
public:
  Session(io_t& io) : m_io(io) {}
  virtual ~Session() {}

  auto connected() const noexcept { return m_connected; }

protected:
  bool  m_connected = false;
  io_t& m_io;
};

template <typename io_t>
class SessionMaster : public Session<io_t>
{
  /* This is neccessary because session is a template class. */
  using Session<io_t>::m_connected;
  using Session<io_t>::m_io;

public:
  SessionMaster(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session<io_t>(io), m_p_address(paddr), m_s_address(saddr)
  {
  }
  virtual ~SessionMaster() {}

  bool test(void)
  {
    const std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD};
    const Frame                cmd(test_data, Frame::Type::TEST, true, m_s_address); // U frame with test  data.
    Frame                      resp;

    if (m_io.send_frame(cmd) == false)
      return false;

    if (m_io.recieve_frame(resp) == false)
      return false;

    if (resp.has_payload() && resp.get_type() == cmd.get_type())
    {
      return (resp.get_payload() == test_data);
    }

    return false;
  }

  void disconnect() { m_connected = false; }

  StatusError connect()
  {
    if (m_connected == false)
    {
      Frame cmd(Frame::Type::SET_NORMAL_RESPONSE_MODE, true, m_s_address); // U frame with test  data.
      if (m_io.send_frame(cmd) == false)
      {
        return StatusError::FailedToSend;
      }

      Frame resp;
      for (;;)
      {
        if (m_io.recieve_frame(resp) == false)
        {
          return StatusError::NoResponse;
        }
        else if (resp.is_final() && resp.get_type() == Frame::Type::UNNUMBERED_ACKNOWLEDGMENT)
        {
          m_connected = true;
          break;
        }
      }
    }

    return StatusError::Success;
  }

private:
  uint8_t m_p_address;
  uint8_t m_s_address;
};

} // namespace hdlc
