/*
 * @Author: lokraszewski
 * @Date:   15-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 28-02-2019
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
    I = 0b00000000,

    RR   = 0b00000001,
    RNR  = 0b00001001,
    REJ  = 0b00000101,
    SREJ = 0b00001101,

    /* Unnumbered types */
    UI      = 0b00000011,
    SNRM    = 0b10000011,
    DISC_RD = 0b01000011,
    UP      = 0b00100011,
    UA      = 0b01100011,
    NR0     = 0b00001011,
    NR1     = 0b10001011,
    NR2     = 0b01001011,
    NR3     = 0b11001011,
    SIM_RIM = 0b00000111,
    FRMR    = 0b10000111,
    SARM_DM = 0b00001111,
    RSET    = 0b10001111,
    SARME   = 0b01001111,
    SNRME   = 0b11001111,
    SABM    = 0b00101111,
    XID     = 0b10101111,
    SABME   = 0b01101111,
    TEST    = 0b11100011,

    UNSET = 0xFF,
  };

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Construct frame with no payload
   *
   * @param[in]  type         Type of frame
   * @param[in]  poll         Whether frame polling
   * @param[in]  address      The address of the frame
   * @param[in]  recieve_seq  The recieve sequence
   * @param[in]  send_seq     The send sequence
   *
   */
  Frame(const Type type = Type::UNSET, const bool poll = true, const uint8_t address = 0xFF, const uint8_t recieve_seq = 0,
        const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111)
  {
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Constructs the frame from a buffer type with begin() end()
   *             iterators.
   *
   * @param[in]  buffer       The buffer
   * @param[in]  type         The type
   * @param[in]  poll         The poll
   * @param[in]  address      The address
   * @param[in]  recieve_seq  The recieve sequence
   * @param[in]  send_seq     The send sequence
   *
   * @tparam     buffer_t     Type of buffer
   *
   */
  template <typename buffer_t>
  Frame(const buffer_t& buffer, const Type type = Type::I, const bool poll = true, const uint8_t address = 0xFF,
        const uint8_t recieve_seq = 0, const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111),
        m_payload(buffer.begin(), buffer.end())
  {
  }

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Construct the frame from a iterator type.
   *
   * @param[in]  begin        The begin
   * @param[in]  end          The end
   * @param[in]  type         The type
   * @param[in]  poll         The poll
   * @param[in]  address      The address
   * @param[in]  recieve_seq  The recieve sequence
   * @param[in]  send_seq     The send sequence
   *
   * @tparam     iter_t       Type of iterator.
   *
   */
  template <typename iter_t>
  Frame(iter_t begin, iter_t end, const Type type = Type::I, const bool poll = true, const uint8_t address = 0xFF,
        const uint8_t recieve_seq = 0, const uint8_t send_seq = 0)
      : m_type(type), m_poll_flag(poll), m_address(address), m_recieve_seq(recieve_seq & 0b111), m_send_seq(send_seq & 0b111),
        m_payload(begin, end)
  {
  }

  void set_address(const uint8_t address) noexcept { m_address = address; }
  auto get_address(void) const noexcept { return m_address; }
  void set_type(const Type type) noexcept { m_type = type; }
  auto get_type() const noexcept { return m_type; }
  void set_poll(bool poll) noexcept { m_poll_flag = poll; }
  void set_recieve_sequence(const uint8_t sequence) noexcept { m_recieve_seq = sequence & 0b111; }
  auto get_recieve_sequence() const noexcept { return (is_unnumbered()) ? 0 : m_recieve_seq; }
  void set_send_sequence(const uint8_t sequence) noexcept { m_send_seq = sequence & 0b111; }
  auto get_send_sequence() const noexcept { return is_information() ? m_send_seq : 0; }

  auto is_payload_type() const noexcept { return m_type == Type::I || m_type == Type::UI || m_type == Type::TEST; }
  auto is_empty() const noexcept { return m_type == Type::UNSET; }
  auto is_valid() const noexcept { return !is_empty(); }
  bool is_information() const noexcept { return m_type == Type::I; }
  bool is_supervisory() const noexcept { return (static_cast<uint8_t>(m_type) & 0b11) == 0b01; }
  bool is_unnumbered() const noexcept { return !is_empty() && (static_cast<uint8_t>(m_type) & 0b11) == 0b11; }

  auto is_poll() const noexcept { return m_poll_flag; }
  auto is_final() const noexcept { return is_poll(); }

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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Comparison operator
   *
   * @param[in]  other  Other frame
   *
   * @return     true if frames are identical
   *
   * @details    Used to check if 2 frames are the same. Useful for unit testing
   *             and loopback tests.
   */
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

  /**
   * @author     lokraszewski
   * @date       28-Feb-2019
   * @brief      Returns true if frames are not the same.
   *
   * @param[in]  other  Other frame
   *
   * @return     true if frames are different
   *
   */
  bool operator!=(const Frame& other) const { return !(*this == other); }

private:
  Type                 m_type        = Type::UNSET; //! Stores the frame type.
  bool                 m_poll_flag   = false;       //! Poll flag
  uint8_t              m_address     = 0xFF;        //! Address
  uint8_t              m_recieve_seq = 0;           //! Receive sequence, only lower 3 bits matter
  uint8_t              m_send_seq    = 0;           //! Send sequence, only lower 3 bits matter.
  std::vector<uint8_t> m_payload;                   //! Payload. Not sure if vector is the correct choice here.
};

} // namespace hdlc
