/*
 * @Author: Lukasz
 * @Date:   20-11-2018
 * @Last Modified by:   Lukasz
 * @Last Modified time: 20-11-2018
 */
#include "hdlc/random_frame_factory.h"
<<<<<<< HEAD

=======
#if HDLC_USE_RANDOM
>>>>>>> feature/options
namespace hdlc
{
std::default_random_engine RandomFrameFactory::m_generator;
}
<<<<<<< HEAD
=======
#endif
>>>>>>> feature/options
