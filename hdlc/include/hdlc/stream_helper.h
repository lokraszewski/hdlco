/*
 * @Author: Lukasz
 * @Date:   16-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 20-11-2018
 */
#pragma once

#include "frame.h"
#include "types.h"


#ifdef __unix__
#include <ostream>

namespace hdlc
{
std::ostream& operator<<(std::ostream& os, const Frame::Type& type);
std::ostream& operator<<(std::ostream& os, const Frame& f);
} // namespace hdlc

#endif
