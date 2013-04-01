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
  const auto it = events.find(trans_id);
  if (it == events.end()) {
	 Event* ev = new Event(true);
	 events.insert
		(std::pair<TransitionId, Event*>(trans_id, ev)); 
      //<NB> manual reset
	 return ev;
  }
  else return it->second;
}

