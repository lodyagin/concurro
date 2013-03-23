#ifndef CONCURRO_ROBJECTWITHSTATES_H_
#define CONCURRO_ROBJECTWITHSTATES_H_

/// An interface which should be implemented in each
/// state-aware class.
template<class StateAxis>
class ObjectWithStatesInterface
{
public:
  virtual ~ObjectWithStatesInterface() {}

  /// set state to the current state of the object
  virtual void state(StateAxis& state) const = 0;

  /// return bool if the object state is state
  virtual bool state_is(const StateAxis& state) const = 0;

protected:

  /// Set the object state without transition cheking (do
  /// not use directly).
  //virtual void set_state_internal 
  // (const StateAxis& state) = 0;
};

/// It can be used as a parent of an object which
/// introduces new state axis.
template<class StateAxis>
class RObjectWithStates 
  : public ObjectWithStatesInterface<StateAxis>
{
public:
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
/// You can inherit from this class to delegate your
/// object state to the StateParent class (which should
/// also be a parent of your class).
template<class StateAxis, class StateParent>
class StatesDelegator 
  : public ObjectWithStatesInterface<StateAxis>,
    virtual public StateParent
{
public:
  /// set state to the current state of the object
  virtual void state(StateAxis& state) const
  {
	 this->StateParent::state(state);
  }

  virtual bool state_is(const StateAxis& state) const
  {
	 StateParent::state_is(state);
  }

protected:

  virtual void set_state_internal (const StateAxis& state)
  {
	 StateParent::set_state_internal(state);
  }
};
#endif

#endif

