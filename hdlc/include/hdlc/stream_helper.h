/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 20-11-2018
 */
#pragma once

#include "frame.h"
#include "options.h"
#include "types.h"
#if HDLC_USE_STREAM_HELPER
#include <ostream>
#endif

namespace hdlc
{
#if HDLC_USE_STREAM_HELPER
std::ostream& operator<<(std::ostream& os, const Frame::Type& type);
std::ostream& operator<<(std::ostream& os, const Frame& f);
#endif
} // namespace hdlc
