/*
 * @Author: lokraszewski
 * @Date:   15-11-2018
 * @Last Modified by:   lokraszewski
 * @Last Modified time: 15-11-2018
 */

#pragma once

#include "types.h"

namespace hdlc
{

class Frame
{
public:
  enum class Type
  {
    UNSET = 0,
    INFORMATION,
    RECEIVE_READY,                  // SUPERVISORY
    RECEIVE_NOT_READY,              // SUPERVISORY
    REJECT,                         // SUPERVISORY
    SELECTIVE_REJECT,               // SUPERVISORY
    UNNUMBERED_INFORMATION,         // UNNUMBERED
    UNNUMBERED_POLL,                // UNNUMBERED
    UNNUMBERED_ACKNOWLEDGMENT,      // UNNUMBERED
    SET_INITIALIZATION_MODE,        // UNNUMBERED
    SET_ASYNCHRONOUS_RESPONSE,      // UNNUMBERED
    SET_NORMAL_RESPONSE,            // UNNUMBERED
    SET_ASYNCHRONOUS_BALANCED_MODE, // UNNUMBERED
    DISCONNECT,                     // UNNUMBERED
    TEST,                           // UNNUMBERED
    EXCHANGE_IDENTIFICATION,        // UNNUMBERED
  };

  Frame() {}

  void set_type(const Type type) noexcept { m_type = type; }
  auto get_type() const noexcept { return m_type; }
  auto is_empty() const noexcept { return m_type == Type::UNSET; }
  bool is_information() const noexcept { return m_type == Type::INFORMATION; }
  bool is_supervisory() const noexcept
  {
    switch (m_type)
    {
    case Type::RECEIVE_READY:
    case Type::RECEIVE_NOT_READY:
    case Type::REJECT:
    case Type::SELECTIVE_REJECT: return true;
    default: return false;
    }
  }
  bool is_unnumbered() const noexcept
  {
    switch (m_type)
    {
    case Type::UNNUMBERED_INFORMATION:
    case Type::UNNUMBERED_POLL:
    case Type::UNNUMBERED_ACKNOWLEDGMENT:
    case Type::SET_INITIALIZATION_MODE:
    case Type::SET_ASYNCHRONOUS_RESPONSE:
    case Type::SET_NORMAL_RESPONSE:
    case Type::SET_ASYNCHRONOUS_BALANCED_MODE:
    case Type::DISCONNECT:
    case Type::TEST:
    case Type::EXCHANGE_IDENTIFICATION: return true;
    default: return false;
    }
  }
  void set_poll(bool poll) noexcept { m_poll_flag = poll; }
  auto is_poll() const noexcept { return m_poll_flag; }
  void set_recieve_sequence(const uint8_t sequence) noexcept { m_recieve_seq = sequence; }
  void set_send_sequence(const uint8_t sequence) noexcept { m_send_seq = sequence; }
  auto get_recieve_sequence() const noexcept { return m_recieve_seq; }
  auto get_send_sequence() const noexcept { return m_send_seq; }
  auto has_payload() const noexcept { return !m_payload.empty(); }

  const std::vector<uint8_t>& get_payload() const { return m_payload; }
  void                        set_payload(const std::vector<unsigned char>& payload) { m_payload = payload; }

private:
  Type                 m_type        = Type::UNSET;
  bool                 m_poll_flag   = false;
  uint8_t              m_address     = 0;
  uint8_t              m_recieve_seq = 0;
  uint8_t              m_send_seq    = 0;
  std::vector<uint8_t> m_payload;
};

} // namespace hdlc
