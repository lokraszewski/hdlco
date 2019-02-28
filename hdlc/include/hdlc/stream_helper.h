/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 28-02-2019
 */
#pragma once

#include "frame.h"
#include "types.h"

#if HDLC_USE_IO_STREAM
#include <ostream>

namespace hdlc
{
std::ostream& operator<<(std::ostream& os, const Frame::Type& type);
std::ostream& operator<<(std::ostream& os, const Frame& f);
std::ostream& operator<<(std::ostream& os, const StatusError& err);
std::ostream& operator<<(std::ostream& os, const ConnectionStatus& status);
} // namespace hdlc

#endif
