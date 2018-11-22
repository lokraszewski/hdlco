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

#include "stream_helper.h"

#include <iostream>
#include <map>
#include <vector>

namespace hdlc
{

class Session
{
public:
  Session(const uint8_t primary, const uint8_t secondary) : m_primary(primary), m_secondary(secondary) {}
  virtual ~Session() {}

  uint8_t          primary() const noexcept { return m_primary; }
  uint8_t          secondary() const noexcept { return m_secondary; }
  void             disconnect() { set_status(ConnectionStatus::Disconnected); }
  bool             connected() const noexcept { return get_status() == ConnectionStatus::Connected; }
  ConnectionStatus get_status() const noexcept { return m_status; }
  void             set_status(ConnectionStatus status)
  {
    switch (status)
    {
    case ConnectionStatus::Connecting:
    case ConnectionStatus::Connected: m_status = status; break;
    default:
      m_status      = ConnectionStatus::Disconnected;
      m_send_seq    = 0;
      m_recieve_seq = 0;
      break;
    }
  }

protected:
  uint8_t          m_primary;
  uint8_t          m_secondary;
  ConnectionStatus m_status      = ConnectionStatus::Disconnected;
  uint8_t          m_send_seq    = 0;
  uint8_t          m_recieve_seq = 0;
};

template <typename io_t>
class SessionMaster : public Session
{
  /* This is neccessary because session is a template class. */

public:
  SessionMaster(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session(paddr, saddr), m_io(io) {}
  virtual ~SessionMaster() {}

  StatusError send_recieve(const Frame& cmd, Frame& resp)
  {

    if (!m_io.send_frame(cmd))
    {
      return StatusError::FailedToSend;
    }

    if (cmd.is_poll())
    {

      for (;;)
      {
        Frame temp(Frame::Type::UNSET);
        if (m_io.recieve_frame(temp) == false)
          return StatusError::NoResponse;
        else if (resp.get_address() != primary())
          return StatusError::InvalidAddress;
        else
        {
          resp = std::move(temp);
          break;
        }
      }
    }

    return StatusError::Success;
  }

  StatusError send_command(const Frame& cmd, Frame& resp)
  {
    auto ret = send_recieve(cmd, resp);

    if (cmd.is_poll())
    {
      switch (resp.get_type())
      {
      case Frame::Type::SARM_DM: ret = StatusError::ConnectionError; break;
      default: break;
      }
    }

    if (ret != StatusError::Success)
      disconnect();

    return ret;
  }

  template <typename buffer_t>
  StatusError send_payload(const buffer_t& buffer)
  {
    const Frame cmd(buffer, Frame::Type::I, true, m_secondary);
    Frame       resp;
    const auto  ret = send_command(cmd, resp);
    if (ret == StatusError::Success)
      std::cout << __FUNCTION__ << ' ' << resp << std::endl;
    return ret;
  }

  StatusError test(void)
  {
    const std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD};
    const Frame                cmd(test_data, Frame::Type::TEST, true, m_secondary);
    Frame                      resp;

    const auto ret = send_command(cmd, resp);

    if (ret == StatusError::Success && cmd != resp)
    {
      return StatusError::InvalidResponse;
    }

    return ret;
  }

  StatusError connect()
  {
    if (!connected())
    {
      const Frame cmd(Frame::Type::SNRM, true, m_secondary);
      Frame       resp;
      auto        ret = send_command(cmd, resp);

      if (ret == StatusError::Success)
      {
        if (resp.get_type() == Frame::Type::UA)
        {
          set_status(ConnectionStatus::Connected);
          ret = StatusError::Success;
        }
        else
        {
          ret = StatusError::InvalidResponse;
        }
      }

      return ret;
    }
    else
    {
      return StatusError::Success;
    }
  }

private:
  io_t& m_io;
};

template <typename io_t>
class SessionClient : public Session
{

  /* need information handlers. */
  /* handle I frame*/
  /* default frame handler. */
  /* Use a map? or just a table?*/

public:
  using handler_t = std::function<StatusError(SessionClient<io_t>&, const Frame&, Frame&)>;

  SessionClient(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session(paddr, saddr), m_io(io)
  {
    install_handler(Frame::Type::SNRM, default_snrm_handler);
    install_handler(Frame::Type::TEST, default_test_handler);
  }
  virtual ~SessionClient() {}

  void install_handler(const Frame::Type type, handler_t handler) { m_handler_map[type] = handler; }
  void uninstall_handler(const Frame::Type type) { m_handler_map[type] = default_handler; }

  StatusError handle(const Frame& cmd, Frame& resp)
  {
    // If we are disconnected then send back SARM_DM unless the frame is a setup frame.
    if (!connected() && cmd.get_type() != Frame::Type::SNRM)
    {
      resp = Frame(Frame::Type::SARM_DM, true, secondary());
      return StatusError::Success;
    }
    else if (m_handler_map.count(cmd.get_type()))
    {
      return m_handler_map[cmd.get_type()](*this, cmd, resp);
    }
    else
    {
      return default_handler(*this, cmd, resp);
    }
  }

  ConnectionStatus run(void)
  {
    /* If we are connected then wait for frames*/
    Frame cmd;
    Frame resp(Frame::Type::UNSET); // Set to empty frame to avoid sending unless set by handler.
    if (m_io.recieve_frame(cmd))
    {
      if (cmd.get_address() == primary())
      {
        /* Check seq*/
        const auto ret = handle(cmd, resp);

        switch (ret)
        {
        case StatusError::Success:
          if (resp.is_valid())
            m_io.send_frame(resp);
          break;

        default: disconnect(); break;
        }
      }
    }
    /* If we are not connected then reject everything except setup. */
    return get_status();
  }

  /* By default if the user does not install a handler this handler will be called.*/
  static StatusError default_handler(SessionClient<io_t>& session, const Frame& cmd, Frame& resp)
  {
    std::cout << __FUNCTION__ << " : " << cmd << std::endl;
    return StatusError::InvalidRequest;
  }

  static StatusError default_snrm_handler(SessionClient<io_t>& session, const Frame& cmd, Frame& resp)
  {
    session.set_status(ConnectionStatus::Connected);
    resp = Frame(Frame::Type::UA, true, session.secondary());
    return StatusError::Success;
  }

  static StatusError default_test_handler(SessionClient<io_t>& session, const Frame& cmd, Frame& resp)
  {
    // Create a frame with identical payload.
    resp = Frame(cmd.get_payload(), Frame::Type::TEST, true, session.secondary());
    return StatusError::Success;
  }

private:
  io_t&                                  m_io;
  std::map<const Frame::Type, handler_t> m_handler_map;
};

} // namespace hdlc
