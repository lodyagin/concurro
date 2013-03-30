// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

template<class Axis>
void RObjectWithStates<Axis>
//
::set_state_internal (const State& state)
{
	std::string fromName;
	if (LOG4CXX_UNLIKELY(this->logger()
								-> isDebugEnabled())) 
	{
	   RState<Axis> from(currentState);
		fromName = from.name();
	}

	currentState = state;

	LOGGER_DEBUG(this->logger(), 
				 "Change state from [" << fromName 
				 << "] to [" << currentState.name() << "]");
}

template<class Axis>
Event* RObjectWithEvents<Axis>
//
::get_event(TransitionId trans_id)
{
  Event* ev = 0;
  const auto it = events.find(trans_id);
  if (it == events.end()) {
	 ev = new Event(true); // <NB> manual reset
	 events.insert
		(std::pair<TransitionId, Event*>(trans_id, ev));
  }
  else 
	 ev = it->second;

  return ev;
}

template<class Axis>
void RObjectWithEvents<Axis>
//
::set_state_internal(const State& state)
{
  Parent::set_state_internal(state);
  const TransitionId trans_id = 
	 this->currentState.state_map
	 -> get_transition_id(this->currentState, state);

  auto it = events.find(trans_id);
  if (it != events.end())
	 it->second->set();
}