/*
 * @Author: Lukasz
 * @Date:   22-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 26-11-2018
 */

#pragma once
#include "frame.h"
#include "io.h"
#include "session.h"
#include "types.h"

namespace hdlc
{
namespace session
{
namespace snrm
{

template <typename io_t>
class Client : public Session
{

  /* need information handlers. */
  /* handle I frame*/
  /* default frame handler. */
  /* Use a map? or just a table?*/

public:
  using handler_t = std::function<StatusError(Client<io_t>&, const Frame&, Frame&)>;

  Client(io_t& io, const uint paddr = 0xFF, const uint8_t saddr = 0xFF) : Session(paddr, saddr), m_io(io)
  {
    install_handler(Frame::Type::SNRM, default_snrm_handler);
    install_handler(Frame::Type::TEST, default_test_handler);
  }
  virtual ~Client() {}

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
  static StatusError default_handler(Client<io_t>& session, const Frame& cmd, Frame& resp)
  {
#if HDLC_USE_IO_STREAM
    std::cout << __FUNCTION__ << " : " << cmd << std::endl;
#endif
    (void)session; // Unused.
    (void)resp;    // Unused.
    return StatusError::InvalidRequest;
  }

  static StatusError default_snrm_handler(Client<io_t>& session, const Frame& cmd, Frame& resp)
  {
    session.set_status(ConnectionStatus::Connected);
    (void)cmd; // Unused.
    resp = Frame(Frame::Type::UA, true, session.secondary());
    return StatusError::Success;
  }

  static StatusError default_test_handler(Client<io_t>& session, const Frame& cmd, Frame& resp)
  {
    (void)session; // unused
    // Create a frame with identical payload.
    resp = Frame(cmd.get_payload(), Frame::Type::TEST, true, session.secondary());
    return StatusError::Success;
  }

private:
  io_t&                                  m_io;
  std::map<const Frame::Type, handler_t> m_handler_map;
};

} // namespace snrm
} // namespace session
} // namespace hdlc
