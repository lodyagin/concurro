#include "RObjectWithStates.h"

RObjectWithStatesBase::~RObjectWithStatesBase()
{
  for (auto ce : subscribers_terminals)
	 ce.wait();
}

void RObjectWithStatesBase
::register_subscriber(RObjectWithStatesBase* sub)
{
  // A guard
  is_changing = true;
  if (is_frozen) {
	 is_changing = false;
	 THROW_PROGRAM_ERROR;
  }

  assert(sub);
  subscribers.insert(sub);
  subscribers_terminals.insert(sub->is_terminal_state());
  assert(subscribers_terminals.size() <= 
			subscribers.size());
  is_changing = false;
}

void RObjectWithStatesBase
::state_changed(AbstractObjectWithStates* object)
{
  // A guard
  is_frozen = true;
  if (is_changing) THROW_PROGRAM_ERROR;

  for (auto sub : subscribers) 
	 sub->state_changed(object);
}

