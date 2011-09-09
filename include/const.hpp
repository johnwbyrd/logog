/**
 * \file const.hpp Constants.
 */

#ifndef __LOGOG_CONST_HPP__
#define __LOGOG_CONST_HPP__

#ifndef LOGOG_FORMATTER_MAX_LENGTH
/** The maximum length of a single line that a formatter may output, in LOGOG_CHAR units. */
#define LOGOG_FORMATTER_MAX_LENGTH ( 1024 * 16 )
#endif

#ifndef LOGOG_DEFAULT_LOG_BUFFER_SIZE
/** The default size of a RingBuffer object for buffering outputs. */
#define LOGOG_DEFAULT_LOG_BUFFER_SIZE ( 4 * 1024 * 1024 )
#endif 

#endif // __LOGOG_CONST_HPP__
