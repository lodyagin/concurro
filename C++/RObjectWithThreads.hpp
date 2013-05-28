// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHTHREADS_HPP_
#define CONCURRO_ROBJECTWITHTHREADS_HPP_

#include "RObjectWithThreads.h"
#include "RThreadRepository.h"
#include <memory>

template<class Object>
RObjectWithThreads<Object>
::RObjectWithThreads
  (std::initializer_list<ThreadPar*> pars)
: destructor_delegate_is_called(false)

{
  for (ThreadPar* par : pars) {
    threads_pars.push(
      std::unique_ptr<ThreadPar>(par));
  }
}

template<class Object>
RObjectWithThreads<Object>
::~RObjectWithThreads()
{
  // A descendant must call destroy() in its
  // destructor. 
  if (!destructor_delegate_is_called)
    THROW_PROGRAM_ERROR;
}

template<class Object>
void RObjectWithThreads<Object>
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object)
{
  if (RAxis<ConstructibleAxis>::state_is(
        *this, complete_constructionState))
  {
    while (!threads_pars.empty()) {
      ThreadPar* par = threads_pars.front().get();
      par->object = dynamic_cast<Object*>(this);
      SCHECK(par->object);
      threads.push_back(
        RThreadRepository<RThread<std::thread>>
        ::instance().create_thread(*par));
      threads_pars.pop();
    }
    for (RThreadBase* th : threads) {
      threads_terminals.push_back(th->is_terminal_state());
      th->start();
    }
  }
  RConstructibleObject::state_changed(ax, state_ax, object);
}

template<class Object>
void RObjectWithThreads<Object>
::destroy()
{
  if (destructor_delegate_is_called) 
    return;

  for (auto& teh : threads_terminals)
    teh.wait();
  for (RThreadBase* th : threads)
    RThreadRepository<RThread<std::thread>>
      ::instance().delete_thread(th);

  destructor_delegate_is_called = true;
}

#endif

