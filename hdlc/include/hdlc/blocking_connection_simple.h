/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 16-11-2018
 */

#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "frame.h"
#include "serializer.h"
#include "types.h"

namespace hdlc
{

class ConnectionBlockingSimple
{
public:
  using read_impl_t  = std::function<StatusError(std::vector<uint8_t> &)>;
  using write_impl_t = std::function<StatusError(std::vector<uint8_t> &)>;
  using sleep_impl_t = std::function<void(const size_t)>;

  ConnectionBlockingSimple(read_impl_t read_impl, write_impl_t write_impl, sleep_impl_t sleep_impl)
      : m_read_impl(read_impl), m_write_impl(write_impl), m_sleep_impl(sleep_impl)
  {
  }
  virtual ~ConnectionBlockingSimple() = default;

  StatusError write(const std::vector<uint8_t> &payload)
  {
    if (payload.empty())
      return StatusError::InvalidParameters;

    std::lock_guard<std::mutex> write_lock(m_write_mutex);

    Frame frame(payload);
    frame.set_recieve_sequence(m_read_sequence);
    frame.set_send_sequence(m_write_sequence);

    auto frame_raw = FrameSerializer::escape(FrameSerializer::serialize(frame));

    for (auto write_attempt = 0; write_attempt < m_write_tries; ++write_attempt)
    {
      auto err = m_write_impl(frame_raw);
      if (err != StatusError::Success)
        return err;

      /* Wait for a response*/
    }

    return StatusError::InvalidParameters;
  }

  StatusError read(std::vector<uint8_t> &payload)
  {

    // do
    // {

    //   // std::vector<uint8_t> read;
    //   auto result = if ((result = transportRead(transportReadBuffer.data(), length)) <= 0) return result;

    //   // Insert the read data into the readBuffer for easier manipulation (e.g. erase)
    //   readBuffer.insert(readBuffer.end(), transportReadBuffer.begin(), transportReadBuffer.begin() + result);
    //   result = decode(readFrame, readSequenceNumber, readBuffer, data, length, discardBytes);

    //   if (discardBytes > 0)
    //     readBuffer.erase(readBuffer.begin(), readBuffer.begin() + discardBytes);

    //   if (result >= 0)
    //   {
    //     switch (readFrame)
    //     {
    //     case FrameData:
    //       if (++readSequenceNumber > 7)
    //         readSequenceNumber = 0;
    //       writeFrame(FrameAck, readSequenceNumber, nullptr, 0);
    //       return result;
    //     case FrameAck:
    //     case FrameNack: writeResult.store(readFrame); break;
    //     }
    //   }
    //   else if ((result == -EIO) && (readFrame == FrameData))
    //   {
    //     writeFrame(FrameNack, readSequenceNumber, nullptr, 0);
    //   }
    // } while (!stopped);

    return StatusError::ErrorFatal;
  }

private:
  std::mutex   m_write_mutex;
  read_impl_t  m_read_impl;
  write_impl_t m_write_impl;
  sleep_impl_t m_sleep_impl;

  uint8_t m_read_sequence  = 0;
  uint8_t m_write_sequence = 0;
  uint8_t m_write_tries    = 1;
};

}; // namespace hdlc
