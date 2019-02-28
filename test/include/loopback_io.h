
#pragma once
#include <chrono>
#include <mutex>
#include <thread>

#include "hdlc/frame.h"
#include "hdlc/hdlc.h"
#include "hdlc/io.h"
#include "hdlc/random_frame_factory.h"
#include "hdlc/serializer.h"
#include "hdlc/stream_helper.h"

namespace hdlc
{

class loopback_io : public base_io
{

public:
  loopback_io()
      : base_io(), t_rx([&]() {
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
  ~loopback_io()
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
    while (m_out_pipe.empty() == false && m_in_pipe.full() == false)
    {
      // Write to the input pipe.
      m_in_pipe.write(m_out_pipe.read());
    }
    return true;
  }

  bool handle_in(void) override { return true; }

  void reset(void) override
  {
    m_out_pipe.clear();
    m_in_pipe.clear();
  }

  void sleep(const size_t ms) override { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

private:
  std::thread        t_rx;
  std::thread        t_tx;
  mutable std::mutex m_end_of_program_mutex;
  bool               m_end_of_program = false;

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
