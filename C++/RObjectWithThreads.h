// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHTHREADS_H_
#define CONCURRO_ROBJECTWITHTHREADS_H_

#include "RThread.h"
#include <list>
#include <queue>

DECLARE_AXIS(ConstructibleAxis, StateAxis);

//! A technical object. Must be moved to the
//! "complete_construction" state in the last derivative,
//! for example, when threads are ready to start (see
//! RObjectWithThreads). 
class RConstructibleObject
  : public RObjectWithEvents<ConstructibleAxis>
{
  DECLARE_EVENT(ConstructibleAxis, complete_construction);

public:
  DECLARE_STATES(ConstructibleAxis, ConstructibleState);
  DECLARE_STATE_CONST(ConstructibleState, in_construction);
  DECLARE_STATE_CONST(ConstructibleState, 
                      complete_construction);

  RConstructibleObject();
};

DECLARE_AXIS(ObjectWithThreadsAxis, ConstructibleAxis);

template<class Object>
struct ThreadOfObjectPar 
  : public RThread<std::thread>::Par
{
public:
  Object* object;
};

template<class Object>
class RObjectWithThreads
: public RConstructibleObject,
  public RStateSplitter
  <ObjectWithThreadsAxis, ConstructibleAxis>
{
public:
  using ThreadPar = ThreadOfObjectPar<Object>;

  RObjectWithThreads(std::initializer_list<ThreadPar*>);

  RObjectWithThreads(const RObjectWithThreads&) = delete;
  virtual ~RObjectWithThreads();

  RObjectWithThreads& operator=
    (const RObjectWithThreads&) = delete;

protected:
  void complete_construction
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override
  {
    THROW_PROGRAM_ERROR;
  }

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  { 
    return RStateSplitter
      <ObjectWithThreadsAxis, ConstructibleAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter
      <ObjectWithThreadsAxis, ConstructibleAxis>
      ::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter
      <ObjectWithThreadsAxis, ConstructibleAxis>
      ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }

  std::queue< std::unique_ptr<ThreadPar> > threads_pars;

  std::list<RThreadBase*> threads;
  std::list<CompoundEvent> threads_terminals;
};

#endif


