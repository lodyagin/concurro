/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file An unified wrapper over different type of threads
 * (i.e., QThread, posix thread etc.).
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREAD_H_
#define CONCURRO_RTHREAD_H_

#include <string>
#include <atomic>
#include <thread>
#include <memory>
#include <type_traits>
#include "SNotCopyable.h"
#include "RMutex.h"
#include "REvent.h"
#include "RState.h"
#include "SCommon.h"
#include "RObjectWithStates.h"
#include "Repository.h"

namespace curr {

/**
 * @addtogroup exceptions
 * @{
 */

struct RThreadException : virtual std::exception {};

//! @}


/**
 * @defgroup threads
 * Threads.
 * @{
 */

struct ThreadAxis;

//! An ancestor of all states of a thread.
DECLARE_AXIS(ThreadAxis, StateAxis);

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular
 * physical thread.
 *
 * @dot
 * digraph {
 *    start [shape = point]; 
 *    terminated [shape = doublecircle];
 *    cancelled [shape = doublecircle];
 *    start -> ready;
 *    ready -> starting [label = "start()"];
 *    starting -> working;
 *    working -> terminated;
 *    ready -> cancelled [label = "cancel()"];
 *    cancelled -> cancelled;
 * }
 * @enddot
 *
 */
class RThreadBase
: public SNotCopyable,
  public HasStringView,
  public RObjectWithEvents<ThreadAxis>
{
  friend struct ThreadAxis;

  DECLARE_EVENT(ThreadAxis, starting);
  DECLARE_EVENT(ThreadAxis, terminated);
  DECLARE_EVENT(ThreadAxis, cancelled);

public:
  struct Par
  {
    //! The type for RThreadBase::Par numeration.
    typedef uint64_t par_num_t;

    Event* extTerminated;

    Par(Event* ext_terminated = 0) :
      extTerminated(ext_terminated),
      par_num([]()
      { 
        static std::atomic<par_num_t> cnt(0);
        return ++cnt;
      }())
    {
    }

    virtual ~Par() {}
    virtual RThreadBase* create_derivation
      (const ObjectCreationInfo&) const = 0;
    virtual RThreadBase* transform_object
      (const RThreadBase*) const = 0;

    //! Number of this par (for better logging)
    const par_num_t par_num;

    std::string thread_name;
  };

  CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }

  //! Contains a common thread execution code. It calls
  //! user-defined run(). It should be protected but is
  //! public for access from a derived RThread template.
  void _run ();

  //! Id of this thread in a repository
  const std::string universal_object_id;
  //! A pretty name for logging (if not empty)
  const std::string thread_name;

  RThreadBase 
    (const std::string& id,
     Event* extTerminated = 0,
     //!< If not null the thread will set this event at
     //! the exit of the run() method.
     //! It is deprecated, use REvent.
     const std::string& name = std::string()
     //!< Name for appearing in log. By default it is id.
      );

  virtual ~RThreadBase ();

  std::string universal_id() const
  {
    return universal_object_id;
  }

  std::string pretty_id() const
  {
    return (thread_name.empty())
      ? universal_object_id : thread_name;
  }

#if 0
  std::string object_name() const override
  {
    return pretty_id();
  }
#endif

  /* 
     It is a group of functions 
     for access from a calling thread 
  */

  // TODO add std::call_once conditions. (? Also to dtr).

  RThreadBase& start();
  virtual void stop (); //!< sets isStopRequested

  //! Move non-started thread to the `cancelled' state and
  //! return true.
  //! Do nothing for started threads and return false.
  virtual bool cancel();

  //! Return true if the state is "working".
  bool is_running () const
  {
    return RAxis<ThreadAxis>::state_is
      (*this, RThreadBase::workingState);
  }


  void outString (std::ostream& out) const override;
  
  //! @cond
  DECLARE_STATES(ThreadAxis, ThreadState);
  DECLARE_STATE_CONST(ThreadState, ready);
  DECLARE_STATE_CONST(ThreadState, starting);
  DECLARE_STATE_CONST(ThreadState, working);
  DECLARE_STATE_CONST(ThreadState, terminated);
  DECLARE_STATE_CONST(ThreadState, cancelled);
  //! @endcond

  //void state(ThreadState& state) const;

  //! @cond
  DEFAULT_LOGGER(RThreadBase);
  //! @endcond

protected:
  struct LogParams {
    static bool current;
  } log_params_;

  bool destructor_delegate_is_called;

  //! stop is requested
  Event isStopRequested; 

  CompoundEvent is_terminal_state_event;

  RThreadBase(const ObjectCreationInfo&, const Par&);

  //! A real work of a destructor is here. All descendants
  //! must call it in the destructor. It waits the
  //! terminationEvent. (Must be called from the most
  //! overrided destructor to prevent destroying of
  //! critical objects prior to _run() exit).
  void destroy();

  //void set_state_internal(const ThreadState& state);

  void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

  //! It will be overrided with real thread procedure.
  virtual void run() = 0;

  virtual void start_impl () = 0;

  //! A number of threads waiting termination
  //! of this thread.
  //TODO add this to (R)Event
  //std::atomic<int> waitCnt; 

private:
  Event* externalTerminated;
};

typedef RThreadBase::ThreadState RThreadState;

/*
  It can be started and stoped only once, like Java
  thread. An exception will be raised if we try to start
  several times.  */

/**
 * Any kind thread-wrapper object. %Thread is the real
 * thread class. Each RThread object must be a member of a
 * ThreadRepository.
 */
template<class Thread> class RThread {};

template<>
class RThread<std::thread> : public RThreadBase
{
public:

  typedef std::thread::native_handle_type ThreadId;
  typedef ThreadId Id;

  class Par;

  //! A RepositoryType to use with this RThread
  typedef RepositoryInterface<
    RThread<std::thread>, Par, ThreadId > RepositoryType;

  class Par : public RThreadBase::Par
  {
  public:
    Par(Event* ext_terminated = 0)
    : RThreadBase::Par(ext_terminated),
      rthreadCreated(
        SFORMAT("StdThread::Par::rthreadCreated:" 
                << par_num << "par"), true, false),
      rthreadStarted(
        SFORMAT("StdThread::Par::rthreadStarted:" 
                << par_num << "par"), true, false),
      repository(0), th_id(0)
    {}

    ~Par()
    {
      rthreadStarted.wait();
    }

    RThreadBase* transform_object
      (const RThreadBase*) const
    { 
      throw ::types::exception
        <TransformObjectIsNotImplemented>();
    }

    std::thread::native_handle_type get_id(
      ObjectCreationInfo& oi
    ) const;

    std::unique_ptr<std::thread>&& move_thread()
    { return std::move(th); }

    void run0()
    {
      assert(repository);
      rthreadCreated.wait();
      assert(th_id);
      RThread<std::thread>* rthread = repository
        -> get_object_by_id(th_id);
      assert(rthread);
      rthreadStarted.set();
      // <NB> `this' can be already destroyed 
      // at this point
      rthread->_run();
    }

  protected:
    mutable std::unique_ptr<std::thread> th;
    mutable Event rthreadCreated;
    Event rthreadStarted;
    mutable RepositoryType* repository;
    mutable std::thread::native_handle_type th_id;

  private:
    typedef Logger<Par> log;
  };

  RThread(const std::string& id, Event* extTerminated = 0)
  : RThreadBase(id, extTerminated),
    th(new std::thread
       (&RThreadBase::_run, this)) {}

  ~RThread() 
  { 
    if (!destructor_delegate_is_called)
      LOG_FATAL(log,
        "Eevery descendant "
        "must call destroy() in its destructor"
      );

    th->join(); 
  }

  //! Create in the repository. 
  template<class Thread>
  static Thread* create
    (const Par& par = typename Thread::Par());

  //! Create in the repository. args0, args... are
  //! parameters to a %Thread::Par constructor.
  template <
    class Thread, 
    class Arg0, 
    class... Args,
    class = typename std::enable_if <
      ! std::is_base_of < 
          Par, 
          typename std::remove_reference<Arg0>::type
        >::value
    >::type
  >
  static Thread* create(Arg0&& arg0, Args&&... args);

  //! Destroy in the repository. 
  //! @param freeMemory Call a destructor (for using with
  //! threads created with create()) 
  void remove(bool freeMemory = true);

  //! Return ptr to the current thread or nullptr if the
  //! current thread is not registered in a global
  //! RThread<std::thread> repository.
  static RThread<std::thread>* current();

  //! A pretty id of the current thread. 
  //! \see current()
  //! \see pretty_id() 
  static std::string current_pretty_id();

  //! Store an id of a current thread as a main thread
  static void this_is_main_thread()
  {
    if (!(main_thread_id == std::thread::id()
          || main_thread_id == std::this_thread::get_id()
          ))
      throw ::types::exception<RThreadException>
        ("this_is_main_thread() was called already");

    main_thread_id = std::this_thread::get_id();
  }

protected:
  //! It is for creation from ThreadRepository
  //! NB use RThread::Par to ensure not mix different
  //! threads models in one repository.
  RThread(const ObjectCreationInfo& oi, const Par& par)
    : RThreadBase(oi, par),
    th(const_cast<Par&>(par).move_thread()) {}

  void start_impl () {}

  std::unique_ptr<std::thread> th;

  static std::thread::id main_thread_id;

  DEFAULT_LOGGER(RThread<std::thread>)
};

typedef RThread<std::thread> StdThread;

/**
 * An easy-to-use thread object for starting small pieces
 * of code in parallel to main execution.
 *
 * Example: StdThread::create<LightThread>([](){ ... })
 *            ->start()
 */
class LightThread : public StdThread
{
public:
  struct Par : public StdThread::Par
  {
    std::function<void()> fun;

    Par(const std::function<void()>& funct,
        const std::string& name = std::string()) 
      : fun(funct) 
    {
      thread_name = name;
    }

    RThreadBase* create_derivation
      (const ObjectCreationInfo& oi) const override
    {
      return new LightThread(oi, *this);
    } 
  };

  ~LightThread() 
  { 
    destroy(); 
    try {
      remove(false); 
    } 
    catch(...) {}
  }

  void run() override;

protected:
  std::function<void()> fun;

  LightThread(const ObjectCreationInfo&, const Par&);
};

/**
 * A more light thread wrapper than ever LightThread.
 * Example: SharedThread([](...){}); // already
 * started, will ROI wait for termination.
 */
class SharedThread
{
public:
  template<class Fun>
  SharedThread(Fun fun) 
    : thread_ptr(StdThread::create<LightThread>(fun)) 
  {
    thread_ptr->start();
  }

  LightThread* operator->()
  {
    return thread_ptr.get();
  }

protected:
  std::shared_ptr<LightThread> thread_ptr;
};

// TODO LocalThread

//! @}

}
#endif
