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

  StatusError send_command(const Frame& cmd, Frame& resp)
  {
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
      else
      {
        // Check if response has sequence, and if so check the sequence.
        if ((resp.is_information() || resp.is_supervisory()) && resp.get_recieve_sequence() != m_send_seq)
        {
          ret = StatusError::InvalidSequence;
        }
        else
        {
          // Check the response.
          switch (resp.get_type())
          {
          // Acknoledge - success.
          case Frame::Type::UA: ret = StatusError::Success; break;
          // Diconnect mode.
          case Frame::Type::SARM_DM: ret = StatusError::ConnectionError; break;
          default: break;
          }
        }
      }

    } while (ret == StatusError::Busy);

    if (ret != StatusError::Success)
      disconnect();

    return ret;
  }

  StatusError test(void)
  {
    const std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC, 0xDD};
    const Frame                cmd(test_data, Frame::Type::TEST, true, m_secondary, m_recieve_seq, m_send_seq++);
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
public:
  SessionClient(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session(paddr, saddr), m_io(io) {}
  virtual ~SessionClient() {}

  void handle_connected(const Frame& cmd) { std::cout << __FUNCTION__ << std::endl; }

  void handle_disconnected(const Frame& cmd)
  {
    std::cout << __FUNCTION__ << std::endl;
    switch (cmd.get_type())
    {
    case Frame::Type::SNRM:
      /* normal response mode requested. Valid operation type so send back UA. */
      {
        /* Set mode to connected. */
        set_status(ConnectionStatus::Connected);
        const Frame resp(Frame::Type::UA, true, m_secondary);
        m_io.send_frame(resp);
      }
      break;
    default:
      /* Send back disconnected. */
      {
        const Frame resp(Frame::Type::SARM_DM, true, m_secondary);
        m_io.send_frame(resp);
      }
      break;
    }
  }

  ConnectionStatus run(void)
  {
    /* If we are connected then wait for frames*/
    Frame cmd;
    if (m_io.recieve_frame(cmd))
    {
      std::cout << "NEW FRAME IN: " << cmd << std::endl;

      if (cmd.get_address() != primary())
      {
        /* Address does not match this session, discard the frame. */
        std::cout << "ADDRESS MISMATCH" << std::endl;
      }
      else
      {
        /* Check seq*/
        std::cout << "ADDRESS MATCH" << std::endl;
        if (connected())
        {
          handle_connected(cmd);
        }
        else
        {
          handle_disconnected(cmd);
        }
      }
    }
    /* If we are not connected then reject everything except setup. */
    return get_status();
  }

private:
  io_t& m_io;
};

} // namespace hdlc
