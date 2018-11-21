/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#include "hdlc/stream_helper.h"

#ifdef __unix__

#include <assert.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace hdlc
{
std::ostream& operator<<(std::ostream& os, const Frame::Type& type)
{
  switch (type)
  {

  case Frame::Type::INFORMATION: os << "INFORMATION"; break;
  case Frame::Type::RECEIVE_READY: os << "RECEIVE_READY"; break;
  case Frame::Type::RECEIVE_NOT_READY: os << "RECEIVE_NOT_READY"; break;
  case Frame::Type::REJECT: os << "REJECT"; break;
  case Frame::Type::SELECTIVE_REJECT: os << "SELECTIVE_REJECT"; break;
  case Frame::Type::UNNUMBERED_INFORMATION: os << "UNNUMBERED_INFORMATION"; break;
  case Frame::Type::SET_ASYNCHRONOUS_BALANCED_MODE: os << "SET_ASYNCHRONOUS_BALANCED_MODE"; break;
  case Frame::Type::UNNUMBERED_ACKNOWLEDGMENT: os << "UNNUMBERED_ACKNOWLEDGMENT"; break;
  case Frame::Type::SET_ASYNCHRONOUS_RESPONSE_MODE: os << "SET_ASYNCHRONOUS_RESPONSE_MODE"; break;
  case Frame::Type::INITIALIZATION: os << "INITIALIZATION"; break;
  case Frame::Type::DISCONNECT: os << "DISCONNECT"; break;
  case Frame::Type::UNNUMBERED_POLL: os << "UNNUMBERED_POLL"; break;
  case Frame::Type::RESET: os << "RESET"; break;
  case Frame::Type::EXCHANGE_IDENTIFICATION: os << "EXCHANGE_IDENTIFICATION"; break;
  case Frame::Type::FRAME_REJECT: os << "FRAME_REJECT"; break;
  case Frame::Type::NONRESERVED0: os << "NONRESERVED0"; break;
  case Frame::Type::NONRESERVED2: os << "NONRESERVED2"; break;
  case Frame::Type::SET_NORMAL_RESPONSE_MODE: os << "SET_NORMAL_RESPONSE_MODE"; break;
  case Frame::Type::NONRESERVED1: os << "NONRESERVED1"; break;
  case Frame::Type::NONRESERVED3: os << "NONRESERVED3"; break;
  case Frame::Type::TEST: os << "TEST"; break;
  case Frame::Type::UNSET: os << "UNSET"; break;

  default: assert(0);
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Frame& f)
{
  if (f.is_empty())
  {
    os << fmt::format("HDLC EMPTY FRAME");
  }
  else
  {
    os << fmt::format("HDLC {} Address: {:#x}, Poll: {}, Send Seq {}, Recieve Seq {}", f.get_type(), f.get_address(), f.is_poll(),
                      f.get_send_sequence(), f.get_recieve_sequence());

    if (f.has_payload())
    {
      const auto payload = f.get_payload();
      os << fmt::format(", {} bytes : ", payload.size());
      for (const auto byte : payload)
      {
        os << fmt::format("{:x} ", byte);
      }
    }
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, const StatusError& err)
{
  switch (err)
  {
  case StatusError::Success: os << "Success"; break;
  case StatusError::InvalidParameters: os << "InvalidParameters"; break;
  case StatusError::ErrorFatal: os << "ErrorFatal"; break;
  case StatusError::FailedToSend: os << "FailedToSend"; break;
  case StatusError::NoResponse: os << "NoResponse"; break;
  case StatusError::InvalidResponse: os << "InvalidResponse"; break;
  case StatusError::InvalidAddress: os << "InvalidAddress"; break;
  default: os << "Unknown"; break;
  }

  return os;
}
} // namespace hdlc

#endif
