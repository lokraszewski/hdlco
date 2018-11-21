/*
 * @Author: lokraszewski
 * @Date:   15-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
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
    UNNUMBERED_INFORMATION         = 0b00000011,
    SET_ASYNCHRONOUS_BALANCED_MODE = 0b00101111,
    UNNUMBERED_ACKNOWLEDGMENT      = 0b01100011,
    SET_ASYNCHRONOUS_RESPONSE_MODE = 0b00001111,
    SET_NORMAL_RESPONSE_MODE       = 0b10000011,
    INITIALIZATION                 = 0b00000111,
    DISCONNECT                     = 0b01000011,
    UNNUMBERED_POLL                = 0b00100011,
    RESET                          = 0b10001111,
    EXCHANGE_IDENTIFICATION        = 0b10101111,
    FRAME_REJECT                   = 0b10000111,
    NONRESERVED0                   = 0b00001011,
    NONRESERVED2                   = 0b01001011,
    NONRESERVED1                   = 0b10001011,
    NONRESERVED3                   = 0b11001011,
    TEST                           = 0b11100011,
    UNSET                          = 0xFF,
  };

  Frame(const Type type = Type::INFORMATION, const bool poll = true, const uint8_t address = 0xFF, const uint8_t recieve_seq = 0,
        const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111)
  {
  }

  template <typename buffer_t>
  Frame(const buffer_t& buffer, const Type type = Type::INFORMATION, const bool poll = true, const uint8_t address = 0xFF,
        const uint8_t recieve_seq = 0, const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111),
        m_payload(buffer.begin(), buffer.end())
  {
  }

  template <typename iter_t>
  Frame(iter_t begin, iter_t end, const Type type = Type::INFORMATION, const bool poll = true, const uint8_t address = 0xFF,
        const uint8_t recieve_seq = 0, const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111),
        m_payload(begin, end)
  {
  }

  void set_address(const uint8_t address) noexcept { m_address = address; }
  auto get_address(void) const noexcept { return m_address; }

  void set_type(const Type type) noexcept { m_type = type; }
  auto get_type() const noexcept { return m_type; }
  auto is_payload_type() const noexcept
  {
    return m_type == Type::INFORMATION || m_type == Type::UNNUMBERED_INFORMATION || m_type == Type::TEST;
  }
  auto is_empty() const noexcept { return m_type == Type::UNSET; }
  bool is_information() const noexcept { return m_type == Type::INFORMATION; }
  bool is_supervisory() const noexcept { return (static_cast<uint8_t>(m_type) & 0b11) == 0b01; }
  bool is_unnumbered() const noexcept { return !is_empty() && (static_cast<uint8_t>(m_type) & 0b11) == 0b11; }
  void set_poll(bool poll) noexcept { m_poll_flag = poll; }
  auto is_poll() const noexcept { return m_poll_flag; }
  auto is_final() const noexcept { return is_poll(); }

  void set_recieve_sequence(const uint8_t sequence) noexcept { m_recieve_seq = sequence & 0b111; }
  void set_send_sequence(const uint8_t sequence) noexcept { m_send_seq = sequence & 0b111; }

  auto get_recieve_sequence() const noexcept { return (is_unnumbered()) ? 0 : m_recieve_seq; }
  auto get_send_sequence() const noexcept { return is_information() ? m_send_seq : 0; }

  auto                        begin() const { return m_payload.begin(); }
  auto                        end() const { return m_payload.end(); }
  auto                        payload_size() const noexcept { return m_payload.size(); }
  const std::vector<uint8_t>& get_payload() const { return m_payload; }
  auto                        has_payload() const noexcept { return !m_payload.empty(); }
  void                        set_payload(const std::vector<unsigned char>& payload) { m_payload = payload; }
  template <typename iter_t>
  void set_payload(iter_t begin, iter_t end)
  {
    m_payload.reserve(end - begin);
    copy(begin, end, std::back_inserter(m_payload));
  }

  bool operator==(const Frame& other) const
  {
    if (get_type() != other.get_type())
      return false;
    if (m_poll_flag != other.is_poll())
      return false;
    if (get_recieve_sequence() != other.get_recieve_sequence())
      return false;
    if (get_send_sequence() != other.get_send_sequence())
      return false;
    if (payload_size() != other.payload_size())
      return false;

    return std::equal(m_payload.begin(), m_payload.end(), other.begin());
  }

  bool operator!=(const Frame& other) const { return !(*this == other); }

private:
  Type                 m_type        = Type::UNSET;
  bool                 m_poll_flag   = false;
  uint8_t              m_address     = 0xFF;
  uint8_t              m_recieve_seq = 0;
  uint8_t              m_send_seq    = 0;
  std::vector<uint8_t> m_payload;
};

} // namespace hdlc
