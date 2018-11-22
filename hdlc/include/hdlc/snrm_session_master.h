/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 22-11-2018
 */

#pragma once

#include "frame.h"
#include "io.h"
#include "session.h"
#include "types.h"

#include "stream_helper.h"

#include <iostream>
#include <map>
#include <vector>

namespace hdlc
{

namespace session
{
namespace snrm
{

template <typename io_t>
class Master : public Session
{
  /* This is neccessary because session is a template class. */

public:
  Master(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session(paddr, saddr), m_io(io) {}
  virtual ~Master() {}

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
} // namespace snrm
} // namespace session
} // namespace hdlc
