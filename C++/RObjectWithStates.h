#ifndef CONCURRO_ROBJECTWITHSTATES_H_
#define CONCURRO_ROBJECTWITHSTATES_H_

template<class StateAxis>
class RObjectWithStates
{
public:
  //RObjectWithStates() {}
  //RObjectWithStates(const char* initial_state);

  RObjectWithStates(const StateAxis& initial_state)
	 : currentState(initial_state) 
  {}

  virtual ~RObjectWithStates() {}

  /// set state to the current state of the object
  virtual void state(StateAxis& state) const
  {
	 state.set_by_universal (currentState);
  }

  virtual bool state_is(const StateAxis& state) const
  {
	 return currentState == state;
  }

protected:

  virtual void set_state_internal (const StateAxis& state)
  {
    currentState = state;
  }

  UniversalState currentState;
};

#if 0
template<class StateAxis>
RObjectWithStates<StateAxis>::RObjectWithStates(const char* initial_state)
{
  StateAxis templ;
  currentState = templ.get_state_map()
}

template<class StateAxis>
RObjectWithStates<StateAxis>::~RObjectWithStates()
{
  delete currentState;
}
#endif

#endif

