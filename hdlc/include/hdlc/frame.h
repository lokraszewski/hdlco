/*
 * @Author: lokraszewski
 * @Date:   15-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 16-11-2018
 */

#pragma once

#include "types.h"
#include <vector>

namespace hdlc
{

class Frame
{
public:
  enum class Type : uint8_t
  {
    INFORMATION                    = 0b00000000,
    RECEIVE_READY                  = 0b00000001,
    RECEIVE_NOT_READY              = 0b00001001,
    REJECT                         = 0b00000101,
    SELECTIVE_REJECT               = 0b00001101,
    SET_NORMAL_RESPONSE_MODE       = 0b10000011,
    SET_ASYNCHRONOUS_RESPONSE_MODE = 0b00001111,
    SET_ASYNCHRONOUS_BALANCED_MODE = 0b00101111,
    SET_INITIALIZATION_MODE        = 0b00000111,
    DISCONNECT                     = 0b01000011,
    UNNUMBERED_ACKNOWLEDGMENT      = 0b01110011,
    DISCONNECT_MODE                = 0b00011111,
    REQUEST_DISCONNECT             = 0b01010011,
    REQUEST_INITIALIZATION         = 0b00010111,
    UNNUMBERED_INFORMATION         = 0b00000011,
    UNNUMBERED_POLL                = 0b00100011,
    RESET                          = 0b10001111,
    EXCHANGE_IDENTIFICATION        = 0b10101111,
    TEST                           = 0b11100011,
    FRAME_REJECT                   = 0b10010111,
    NONRESERVED0                   = 0b00001011,
    NONRESERVED1                   = 0b10001011,
    NONRESERVED2                   = 0b01001011,
    NONRESERVED3                   = 0b11001011,
    UNSET                          = 0xFF,
  };

  Frame() {}

  void set_address(const uint8_t address) noexcept { m_address = address; }
  auto get_address(void) const noexcept { return m_address; }

  void set_type(const Type type) noexcept { m_type = type; }
  auto get_type() const noexcept { return m_type; }
  auto is_payload_type() const noexcept { return m_type == Type::INFORMATION || m_type == Type::UNNUMBERED_INFORMATION; }
  auto is_empty() const noexcept { return m_type == Type::UNSET; }
  bool is_information() const noexcept { return m_type == Type::INFORMATION; }
  bool is_supervisory() const noexcept { return (static_cast<uint8_t>(m_type) & 0b11) == 0b01; }
  bool is_unnumbered() const noexcept { return !is_empty() && (static_cast<uint8_t>(m_type) & 0b11) == 0b11; }
  void set_poll(bool poll) noexcept { m_poll_flag = poll; }
  auto is_poll() const noexcept { return m_poll_flag; }
  void set_recieve_sequence(const uint8_t sequence) noexcept { m_recieve_seq = sequence; }
  void set_send_sequence(const uint8_t sequence) noexcept { m_send_seq = sequence; }
  auto get_recieve_sequence() const noexcept { return m_recieve_seq; }
  auto get_send_sequence() const noexcept { return m_send_seq; }
  auto has_payload() const noexcept { return !m_payload.empty(); }

  const std::vector<uint8_t>& get_payload() const { return m_payload; }
  void                        set_payload(const std::vector<unsigned char>& payload) { m_payload = payload; }

  auto begin() const { return m_payload.begin(); }
  auto end() const { return m_payload.end(); }

private:
  Type                 m_type        = Type::UNSET;
  bool                 m_poll_flag   = false;
  uint8_t              m_address     = 0;
  uint8_t              m_recieve_seq = 0;
  uint8_t              m_send_seq    = 0;
  std::vector<uint8_t> m_payload;
};

} // namespace hdlc
