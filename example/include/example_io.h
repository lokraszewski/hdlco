/*
 * @Author: Lukasz
 * @Date:   21-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 21-11-2018
 */

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable> // std::condition_variable
#include <cstdio>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <random>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <serial/serial.h>

#include "hdlc/frame.h"
#include "hdlc/frame_reciever.h"
#include "hdlc/hdlc.h"
#include "hdlc/io.h"
#include "hdlc/random_frame_factory.h"
#include "hdlc/serializer.h"
#include "hdlc/stream_helper.h"

namespace hdlc
{

class example_io : public base_io
{

public:
  example_io(std::shared_ptr<serial::Serial> ptr)
      : base_io(), m_ptr(ptr), t_rx([&]() {
          while (!is_done())
          {
            handle_in();
          }
        }),
        t_tx([&]() {
          while (!is_done())
          {
            handle_out();
          }
        })
  {
  }
  ~example_io()
  {
    done();
    t_rx.join();
    t_tx.join();
  }

  size_t get_tick(void) const override
  {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return static_cast<size_t>(now);
  }
  bool handle_out(void) override
  {
    bool success = true;
    while (m_out_pipe.empty() == false && success)
    {
      auto byte = m_out_pipe.read();
      success   = (bool)m_ptr->write(&byte, 1);
    }
    return success;
  }
  bool handle_in(void) override
  {
    const auto readable = m_ptr->waitReadable();
    if (readable)
    {
      uint8_t byte;
      while (m_in_pipe.full() == false && m_ptr->read(&byte, 1))
      {
        m_in_pipe.write(byte);
      }
    }
    return readable;
  }

private:
  std::shared_ptr<serial::Serial> m_ptr;
  std::thread                     t_rx;
  std::thread                     t_tx;
  mutable std::mutex              m_end_of_program_mutex;
  bool                            m_end_of_program = false;

  bool is_done() const
  {
    std::lock_guard<std::mutex> lock(m_end_of_program_mutex);
    return m_end_of_program;
  }

  void done()
  {
    std::lock_guard<std::mutex> lock(m_end_of_program_mutex);
    m_end_of_program = true;
  }
};
} // namespace hdlc
