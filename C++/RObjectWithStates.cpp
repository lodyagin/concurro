#include "RObjectWithStates.h"

RObjectWithStatesBase::RObjectWithStatesBase()
  : is_frozen(false), is_changing(false)
{}

RObjectWithStatesBase
::RObjectWithStatesBase(RObjectWithStatesBase&& o)
{
  //<NB> no parent
  *this = std::move(o);
}

RObjectWithStatesBase::~RObjectWithStatesBase()
{
  for (auto ce : subscribers_terminals)
	 ce.wait();
}

RObjectWithStatesBase& RObjectWithStatesBase
::operator=(RObjectWithStatesBase&& o)
{
  SCHECK(o.is_frozen);
  is_frozen = true;
  is_changing = false;
  subscribers = std::move(o.subscribers);
  subscribers_terminals =
    std::move(o.subscribers_terminals);
  return *this;
}

void RObjectWithStatesBase
::register_subscriber
  (RObjectWithStatesBase* sub,
   StateAxis* ax
  )
{
  // A guard
  is_changing = true;
  if (is_frozen) {
	 is_changing = false;
	 THROW_PROGRAM_ERROR;
  }

  assert(sub);
  subscribers.insert(std::make_pair(sub, ax));
  subscribers_terminals.insert
	 (std::move(sub->is_terminal_state()));
  assert(subscribers_terminals.size() <= 
			subscribers.size());
  is_changing = false;
}

void RObjectWithStatesBase
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object)
{
  // A guard
  is_frozen = true;
  if (is_changing) 
	 THROW_PROGRAM_ERROR;

  for (auto sub : subscribers) 
    sub.first->state_changed(*sub.second, state_ax, object);
}

