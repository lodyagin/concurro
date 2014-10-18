/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

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
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHTHREADS_HPP_
#define CONCURRO_ROBJECTWITHTHREADS_HPP_

#include "RObjectWithThreads.h"
#include "RThreadRepository.h"
#include <memory>

namespace curr {

template<class Object>
RObjectWithThreads<Object>
::RObjectWithThreads
  (std::initializer_list<ThreadPar*> pars)
: destructor_delegate_is_called(false)
{
  for (ThreadPar* par : pars) {
    LOG_DEBUG(log, "push " << par->par_num << "par to " 
              << curr::type<Object>::name());
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
   AbstractObjectWithStates* object,
   const UniversalState& new_state
  )
{
  if (!ConstructibleAxis::is_same(ax))
    return;

  const RState<ConstructibleAxis> newst(new_state);
  if (newst == complete_constructionState)
  {
    auto& threp = StdThreadRepository::instance();
    while (!threads_pars.empty()) {
      ThreadPar* par = threads_pars.front().get();

      par->object = dynamic_cast<Object*>(this);
      SCHECK(par->object);
      // add a pretty thread id
      par->thread_name = SFORMAT
        (par->object->object_name() << ":"
         << threp.allocate_new_object_id(*par));

      threads.push_back(threp.create_thread(*par));
      threads_pars.pop();
    }
    for (RThreadBase* th : threads) {
      threads_terminals.push_back(th->is_terminal_state());
      th->start();
    }
  }
  ConstructibleObject::state_changed
    (ax, state_ax, object, new_state);
}

template<class Object>
void RObjectWithThreads<Object>
::destroy()
{
  if (destructor_delegate_is_called) 
    return;

  for (RThreadBase* th : threads)
    th->stop();

  for (auto& teh : threads_terminals)
  {
    teh.wait();
  }
  for (RThreadBase* th : threads)
    StdThreadRepository
      ::instance().delete_thread(th);

  destructor_delegate_is_called = true;
}

}
#endif

