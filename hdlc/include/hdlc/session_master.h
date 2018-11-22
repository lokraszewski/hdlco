/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 22-11-2018
 */

#pragma once

#include "frame.h"
#include "io.h"
#include "types.h"

#include <vector>

namespace hdlc
{

enum class ConnectionStatus
{
  Disconnected,
  Connecting,
  Connected,
};

template <typename io_t>
class Session
{
public:
  Session(io_t& io, const uint8_t primary, const uint8_t secondary) : m_io(io), m_primary(primary), m_secondary(secondary) {}
  virtual ~Session() {}

  auto connected() const noexcept { return m_connected; }

  auto primary() const noexcept { return m_primary; }
  auto secondary() const noexcept { return m_secondary; }
  void reset()
  {
    m_send_seq    = 0;
    m_recieve_seq = 0;
    m_connected   = false;
    m_io.reset();
  }

protected:
  bool  m_connected = false;
  io_t& m_io;

  uint8_t m_primary;
  uint8_t m_secondary;
  uint8_t m_send_seq    = 0;
  uint8_t m_recieve_seq = 0;
};

template <typename io_t>
class SessionMaster : public Session<io_t>
{
  /* This is neccessary because session is a template class. */
  using Session<io_t>::m_connected;
  using Session<io_t>::m_io;
  using Session<io_t>::m_primary;
  using Session<io_t>::m_secondary;

public:
  SessionMaster(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session<io_t>(io, paddr, saddr) {}
  virtual ~SessionMaster() {}

  StatusError send_command(const Frame& cmd, Frame& resp)
  {
    cmd.set_send_sequence(m_send_seq++);

    if (!m_io.send_frame(cmd))
    {
      return StatusError::FailedToSend;
    }

    StatusError ret = StatusError::Busy;

    do
    {
      if (!m_io.recieve_frame(resp))
        ret = StatusError::NoResponse;
      else if (!resp.is_final())
        ret = StatusError::Busy;
      else if (resp.get_address() != m_primary)
        ret = StatusError::InvalidAddress;
      else if (cmd.is_poll() && resp.get_recieve_sequence() != m_send_seq)
        ret = StatusError::InvalidSequence;
      else if (resp.get_type() == Frame::Type::UNNUMBERED_ACKNOWLEDGMENT)
        ret = StatusError::Success;
      else
        ret = StatusError::InvalidResponse;
    } while (ret == StatusError::Busy);

    if (ret != StatusError::Success)
      m_connected = false;

    return ret;
  }

  StatusError test(void)
  {
    const std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD};
    const Frame                cmd(test_data, Frame::Type::TEST, true, m_secondary); // U frame with test  data.
    Frame                      resp;

    auto ret = send_command(cmd, resp);

    if (ret == StatusError::Success)
    {
      if (!resp.has_payload() || resp.get_type() != cmd.get_type() || resp.get_payload() != test_data)
      {
        ret = StatusError::InvalidResponse;
      }
    }

    return ret;
  }

  void disconnect() { m_connected = false; }

  StatusError connect()
  {
    if (m_connected == false)
    {
      const Frame cmd(Frame::Type::SET_NORMAL_RESPONSE_MODE, true, m_secondary);
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
        else if (resp.is_final() == false)
        {
          continue;
        }
        else if (resp.get_address() != m_primary)
        {
          return StatusError::InvalidAddress;
        }
        else if (resp.get_type() == Frame::Type::UNNUMBERED_ACKNOWLEDGMENT)
        {
          m_connected = true;
          break;
        }
        else
        {
          return StatusError::InvalidResponse;
        }
      }
    }

    return StatusError::Success;
  }
};

template <typename io_t>
class SessionClient : public Session<io_t>
{
  /* This is neccessary because session is a template class. */
  using Session<io_t>::m_connected;
  using Session<io_t>::m_io;
  using Session<io_t>::m_primary;
  using Session<io_t>::m_secondary;

public:
  SessionClient(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session<io_t>(io, paddr, saddr) {}
  virtual ~SessionClient() {}

  void run(void)
  {
    /* If we are connected then wait for frames*/

    /* If we are not connected then reject everything except setup. */
  }
};

} // namespace hdlc
