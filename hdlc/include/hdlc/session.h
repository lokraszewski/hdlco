/*
 * @Author: Lukasz
 * @Date:   22-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 22-11-2018
 */
#pragma once

#include "frame.h"
#include "io.h"
#include "types.h"

namespace hdlc
{
namespace session
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
} // namespace session
} // namespace hdlc
