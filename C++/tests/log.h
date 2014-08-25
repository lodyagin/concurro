// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef CONCURRO_TESTS_LOG_H
#define CONCURRO_TESTS_LOG_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>
#if 0
#include "util.h"
#endif
#include "types/string.h"
#include "types/time.h"

namespace lg {

#if 0
template<class CharT>
class log_traits;

template<>
class log_traits<char> : public std::char_traits<char>
{
public:
  constexpr static types::constexpr_string 
  truncated_mark()
  {
    return types::constexpr_string("*truncated*\n");
  }
  
  constexpr static char tab()
  {
    return '\t';
  }

  constexpr static char space()
  {
    return ' ';
  }
};

template <
  class CharT,
  class Traits = std::char_traits<CharT>
>
class printf_basic_stream 
  : virtual public std::basic_ios<CharT, Traits>
{
public:
  printf_basic_stream()
    : fLogTimestamps(GetBoolArg("-logtimestamps", true))
  {}

  //! @deprecated
  void vprintf(const CharT* fmt, va_list ap) noexcept;

  bool log_timestamps() const
  {
    return fLogTimestamps;
  }

protected:
  bool fStartedNewLine = true;
  bool fLogTimestamps;

  virtual boost::mutex::scoped_lock lock() 
  {
    return boost::mutex::scoped_lock(); // it means no lock
  }

  virtual void sentried(const std::function<void()>&) = 0;
};
#endif

//! The stream to log. It is a singleton.
class stream 
  : //public printf_basic_stream<char>,
    public boost::filesystem::basic_ofstream<char>
{
public:
  static stream& instance()
  {
    static boost::once_flag of = BOOST_ONCE_INIT;
    static stream* instance = nullptr;
    boost::call_once([](){ instance = new stream(); }, of);
    assert(instance);
    return *instance;
  }

  static int64_t event_id;

protected:
  boost::mutex::scoped_lock lock() //override
  {
    return boost::mutex::scoped_lock(mx);
  }

  void sentried(const std::function<void()>& f) //override
  {
    sentry s(*this);
    if (!s)
      return;

    f();
  }

private:
  stream() 
   : boost::filesystem::ofstream(
       /*GetDataDir() / */ "debug.log",
       std::ios_base::app
     )
  {
    exceptions(std::ios_base::goodbit);
  }

  boost::mutex mx;
};

#if 0
template <class CharT, class Traits>
void printf_basic_stream<CharT, Traits>
//
::vprintf(const CharT* fmt, va_list ap) noexcept
{
  using Log = log_traits<CharT>;
  sentried([this, fmt, ap]()
  {
    boost::mutex::scoped_lock scoped_lock = lock();

    // Debug print useful for profiling
    if (fLogTimestamps && fStartedNewLine)
    {
      ::times::put_time(
        *this, 
        ::times::timestamp<std::chrono::system_clock>
          ("%Y-%m-%d %H:%M:%S")
      );
      this->rdbuf()->sputc(Log::space());
    }

    if (fmt[strlen(fmt) - 1] == '\n')
      fStartedNewLine = true;
    else
      fStartedNewLine = false;

    std::array<CharT, 512> buf;

    const size_t len = vsnprintf(
      buf.data(), 
      buf.size() - Log::truncated_mark().size() + 1, 
      fmt, 
      ap
    );

    // warn that the output is truncated
    if (len >= 
        buf.size() - Log::truncated_mark().size()
        )
    {
      Traits::copy(
        buf.end() - Log::truncated_mark().size(),
        Log::truncated_mark().begin(),
        Log::truncated_mark().size()
      );
    }

    this->rdbuf()->sputn(
      buf.data(), 
      std::min(len, buf.size())
      );
  });
}
#endif

} // lg

//! Continues a log line
inline lg::stream& LOG_()
{
  return lg::stream::instance();
}

//! Starts a log line
inline lg::stream& LOG()
{
  lg::stream& out = lg::stream::instance();
  auto a= ::times::timestamp<std::chrono::system_clock>
    ("%Y-%m-%d %H:%M:%S"); out << a << ' ';
  return out;
}

#endif
