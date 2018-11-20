/*
 * @Author: Lukasz
 * @Date:   20-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 20-11-2018
 */

#pragma once

#include "frame.h"
#include "types.h"

#include <algorithm>
#include <iostream>
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

    return Frame(payload, Frame::Type::INFORMATION, poll, address, recieve_seq, send_seq);
  }

  static Frame make_unnumbered(void)
  {
    static const Frame::Type frame_type[] = {Frame::Type::UNNUMBERED_INFORMATION,
                                             Frame::Type::SET_ASYNCHRONOUS_BALANCED_MODE,
                                             Frame::Type::UNNUMBERED_ACKNOWLEDGMENT,
                                             Frame::Type::SET_ASYNCHRONOUS_RESPONSE_MODE,
                                             Frame::Type::INITIALIZATION,
                                             Frame::Type::DISCONNECT,
                                             Frame::Type::UNNUMBERED_POLL,
                                             Frame::Type::RESET,
                                             Frame::Type::EXCHANGE_IDENTIFICATION,
                                             Frame::Type::FRAME_REJECT,
                                             Frame::Type::NONRESERVED0,
                                             Frame::Type::NONRESERVED2,
                                             Frame::Type::SET_NORMAL_RESPONSE_MODE,
                                             Frame::Type::NONRESERVED1,
                                             Frame::Type::NONRESERVED3,
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
        Frame::Type::RECEIVE_READY,
        Frame::Type::RECEIVE_NOT_READY,
        Frame::Type::REJECT,
        Frame::Type::SELECTIVE_REJECT,
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
