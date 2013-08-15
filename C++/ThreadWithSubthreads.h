/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_THREADWITHSUBTHREADS_H_
#define CONCURRO_THREADWITHSUBTHREADS_H_

#include "RThread.h"
#include "ThreadRepository.h"
#include "Logging.h"
#include <algorithm>

// SubthreadParameter is a parameter
// for subthread creation

template<class Thread, class ThreadParameter>
class ThreadWithSubthreads 
  : public Thread,
    public ThreadRepository<Thread, ThreadParameter>
{
public:

  // parameter to constructor
  typedef unsigned ConstrPar;

  ~ThreadWithSubthreads () {}

  static ThreadWithSubthreads& current ()
  { 
    return reinterpret_cast<ThreadWithSubthreads&>
      (Thread::current ());
  }

  virtual Thread* create_subthread
    (const ThreadParameter& pars)
  {
    return create_object (pars);
  }

  // Overrides
  // Wait termination of all subthreads
  void wait () 
  { 
    this->wait_subthreads(); 
    Thread::wait (); 
  }

  // Overrides
  // Request stop all subthreads and this thread
  void stop () 
  { 
    this->stop_subthreads(); 
    Thread::stop (); 
  }

  // Overrides
  void outString (std::ostream& out) const;

protected:
  ThreadWithSubthreads 
    (const std::string& id,
	  REvent* connectionTerminated,
     unsigned nThreadsMax
     ) 
    : Thread (id, connectionTerminated),
      ThreadRepository <Thread, ThreadParameter> 
	 (id + "_subthreads_rep", nThreadsMax)
  {
    this->log_from_constructor ();
  }
};

template<class Thread>
class OutThread 
  : public std::unary_function<const Thread&, void>
{
public:
  OutThread (std::ostream& _out) : out (_out) {}
  void operator () (const Thread& thread)
  {
    out << '\t';
    thread.outString (out);
    out << '\n';
  }
protected:
  std::ostream& out;
};

template<class Thread, class ThreadParameter>
void ThreadWithSubthreads<Thread, ThreadParameter>::
  outString (std::ostream& out) const
{
  out << "ThreadWithSubthreads (";
#if 0
  this->for_each (OutThread<Thread> (out));
#else
  out << "FIXME"; //FIXME
#endif
  out << ")\n";
}

#endif
