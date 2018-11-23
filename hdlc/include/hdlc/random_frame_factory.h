/*
 * @Author: Lukasz
 * @Date:   20-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 22-11-2018
 */

#pragma once

#include "frame.h"
#include "types.h"

#include <algorithm>
#include <random>

namespace hdlc
{
class RandomFrameFactory
{
public:
  RandomFrameFactory()  = delete;
  ~RandomFrameFactory() = delete;

  static Frame make_inforamtion(const size_t max_payload_length = 512)
  {
    std::vector<uint8_t> payload     = get_random_payload(max_payload_length);
    const bool           poll        = (get_random_byte() & 1) ? true : false;
    const auto           address     = get_random_byte();
    const auto           recieve_seq = get_random_byte();
    const auto           send_seq    = get_random_byte();

    return Frame(payload, Frame::Type::I, poll, address, recieve_seq, send_seq);
  }

  static Frame make_unnumbered(void)
  {
    static const Frame::Type frame_type[] = {Frame::Type::UI,
                                             Frame::Type::SABM,
                                             Frame::Type::UA,
                                             Frame::Type::SARM_DM,
                                             Frame::Type::SIM_RIM,
                                             Frame::Type::DISC_RD,
                                             Frame::Type::UP,
                                             Frame::Type::RSET,
                                             Frame::Type::XID,
                                             Frame::Type::FRMR,
                                             Frame::Type::NR0,
                                             Frame::Type::NR2,
                                             Frame::Type::SNRM,
                                             Frame::Type::NR1,
                                             Frame::Type::NR3,
                                             Frame::Type::TEST};

    static std::uniform_int_distribution<uint8_t> distribution(0, 15);

    const bool poll        = (get_random_byte() & 1) ? true : false;
    const auto address     = get_random_byte();
    const auto recieve_seq = get_random_byte();
    const auto type        = frame_type[distribution(m_generator)];

    return Frame(type, poll, address, recieve_seq);
  }

  static Frame make_supervisory(void)
  {
    static const Frame::Type frame_type[] = {
        Frame::Type::RR,
        Frame::Type::RNR,
        Frame::Type::REJ,
        Frame::Type::SREJ,
    };
    static std::uniform_int_distribution<uint8_t> distribution(0, 3);
    const bool                                    poll        = (get_random_byte() & 1) ? true : false;
    const auto                                    address     = get_random_byte();
    const auto                                    recieve_seq = get_random_byte();
    const auto                                    type        = frame_type[distribution(m_generator)];
    return Frame(type, poll, address, recieve_seq);
  }

  static Frame make(void)
  {
    static std::uniform_int_distribution<uint8_t> distribution(0, 2);
    switch (distribution(m_generator))
    {
    case 0: return make_inforamtion();
    case 1: return make_unnumbered();
    default: return make_supervisory();
    }
  }

  static std::vector<uint8_t> get_random_payload(const size_t max_payload_length = 512)
  {
    auto                 payload_size = get_random(1, max_payload_length);
    std::vector<uint8_t> payload(payload_size);
    std::generate(payload.begin(), payload.end(), [] { return get_random_byte(); });
    return payload;
  }
  static size_t get_random(const size_t min = 0, const size_t max = 512)
  {
    std::uniform_int_distribution<size_t> distribution(min, max);
    return distribution(m_generator);
  }

  static uint8_t get_random_byte(void)
  {
    static std::uniform_int_distribution<uint8_t> m_byte_distribution(0, 0xFF);
    return m_byte_distribution(m_generator);
  }

  static std::default_random_engine m_generator;
};
} // namespace hdlc
