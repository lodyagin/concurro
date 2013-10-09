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
  ConstructibleObject::state_changed
    (ax, state_ax, object, new_state);
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

}
#endif

