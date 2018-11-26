/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 26-11-2018
 */

#include "hdlc/stream_helper.h"

#if HDLC_USE_IO_STREAM

#include <assert.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace hdlc
{
std::ostream& operator<<(std::ostream& os, const Frame::Type& type)
{
  switch (type)
  {
  case Frame::Type::I: os << "information"; break;
  case Frame::Type::RR: os << "receive ready"; break;
  case Frame::Type::RNR: os << "receive not ready"; break;
  case Frame::Type::REJ: os << "reject"; break;
  case Frame::Type::SREJ: os << "selective reject"; break;
  case Frame::Type::UI: os << "unnumbered information"; break;
  case Frame::Type::SABM: os << "set asynchronous balanced mode"; break;
  case Frame::Type::UA: os << "unnumbered acknowledgment"; break;
  case Frame::Type::SARM_DM: os << "set asynchronous response mode / disconnect mode"; break;
  case Frame::Type::SIM_RIM: os << "set / request initialization mode "; break;
  case Frame::Type::DISC_RD: os << "<request> disconnect"; break;
  case Frame::Type::UP: os << "unnumbered poll"; break;
  case Frame::Type::RSET: os << "reset"; break;
  case Frame::Type::XID: os << "exchange identification"; break;
  case Frame::Type::FRMR: os << "frame reject"; break;
  case Frame::Type::NR0: os << "nonreserved0"; break;
  case Frame::Type::NR2: os << "nonreserved2"; break;
  case Frame::Type::SNRM: os << "set normal response mode"; break;
  case Frame::Type::NR1: os << "nonreserved1"; break;
  case Frame::Type::NR3: os << "nonreserved3"; break;
  case Frame::Type::TEST: os << "test"; break;
  case Frame::Type::UNSET: os << "unset"; break;
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

    if (f.is_information())
    {
      os << fmt::format("HDLC {} Address: {:#x}, Poll: {}, Send {}, Recieve {}", f.get_type(), f.get_address(), f.is_poll(),
                        f.get_send_sequence(), f.get_recieve_sequence());
    }
    else if (f.is_supervisory())
    {
      os << fmt::format("HDLC {} Address: {:#x}, Poll: {}, Recieve {}", f.get_type(), f.get_address(), f.is_poll(),
                        f.get_recieve_sequence());
    }
    else if (f.is_unnumbered())
    {
      os << fmt::format("HDLC {} Address: {:#x}, Poll: {}", f.get_type(), f.get_address(), f.is_poll());
    }

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
  case StatusError::FailedToSend: os << "FailedToSend"; break;
  case StatusError::NoResponse: os << "NoResponse"; break;
  case StatusError::InvalidResponse: os << "InvalidResponse"; break;
  case StatusError::InvalidAddress: os << "InvalidAddress"; break;
  case StatusError::InvalidSequence: os << "InvalidSequence"; break;
  case StatusError::ConnectionError: os << "ConnectionError"; break;
  case StatusError::InvalidRequest: os << "InvalidRequest"; break;
  default: os << "Unknown"; break;
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, const ConnectionStatus& status)
{
  switch (status)
  {
  case ConnectionStatus::Disconnected: os << "Disconnected"; break;
  case ConnectionStatus::Connecting: os << "Connecting"; break;
  case ConnectionStatus::Connected: os << "Connected"; break;
  default: os << "Unknown"; break;
  }
  return os;
}

} // namespace hdlc

#endif
