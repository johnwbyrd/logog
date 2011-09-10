/**
 * \file logog.hpp Main include file for logog logging functionality.  Include this file to enable logging for your application.
 */

#ifndef __LOGOG_HPP__
#define __LOGOG_HPP__

#include "const.hpp"
#include "platform.hpp"
#include "statics.hpp"
#include "object.hpp"
#include "timer.hpp"
#include "mutex.hpp"
#include "string.hpp"
#include "node.hpp"
#include "topic.hpp"
#include "formatter.hpp"
#include "target.hpp"
// #include "socket.hpp"
#include "checkpoint.hpp"
#include "api.hpp"
#include "message.hpp"
#include "macro.hpp"

#ifdef LOGOG_UNIT_TESTING
#include "thread.hpp"
#include "unittest.hpp"
#endif

#endif // __LOGOG_HPP_
