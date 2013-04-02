// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

template<class Axis>
REvent<Axis>::REvent(const char* from, 
							const char* to)
  : UniversalEvent
  	   (StateMapRepository::instance()
		 . get_map_for_axis(typeid(Axis))
		 -> get_transition_id(from, to)
		  )
{}

template<class Axis>
REvent<Axis>::REvent(const char* to)
  : UniversalEvent
  	   (StateMapRepository::instance()
	    . get_map_for_axis(typeid(Axis))
	      -> create_state(to)
		  )
{}

template<class Axis>
Event& REvent<Axis>
//
::event(RObjectWithEvents<Axis>& obj)
{
  return *obj.get_event(*this);
}

/*
template<class Axis>
bool REvent<Axis>
//
::wait(RObjectWithEvents<Axis>& obj, int time)
{
  obj.get_event
	 (this->transition_id ) -> wait(time);
}
*/
