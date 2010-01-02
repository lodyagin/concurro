#include "StdAfx.h"
#include "StateMap.h"

Logging StateMap::log ("StateMap");

StateMap::StateMap 
    (const State2Idx new_states[], 
     const StateTransition transitions[]
    )
{
  if (new_states == NULL || transitions == NULL)
    throw BadParameters ("1");

  StateIdx maxIdx = 0;
  {
    // the last element of new_states should have
    // idx = 0
    StateIdx i;
    for (i = 0; new_states[i].idx != 0; i++)
    {
      name2idx[new_states[i].state] = new_states[i].idx;

      if (new_states[i].idx < 1)
        throw BadParameters ("2"); 

      idx2name[new_states[i].idx] = new_states[i].state;

      if (new_states[i].idx > maxIdx)
        maxIdx = new_states[i].idx;
    }
    if (maxIdx != name2idx.size ())
      throw BadParameters ("3"); 
    
    if (maxIdx != i)
      throw BadParameters ("4"); 
  }
  const StateIdx nStates = maxIdx;

  LOG4STRM_DEBUG 
    (log.GetLogger (), 
     oss_ << "Total " << nStates << " states in the map."
     );

  int nTransitions = 0;
  {
    // Count the number of transitions
    // The last element of transitions should be NULL->NULL
    // transition
    int i;
    for (i = 0; ; i++) 
     {
       if (transitions[i].from == NULL
           || transitions[i].to == NULL)
       {
         if (transitions[i].from == NULL
             && transitions[i].to == NULL)
           break;
         else throw BadParameters ("5");
       }
     }
    nTransitions = i;
  }

  //initialize transitions
  trans2number.resize (nStates);
  for (Trans2Number::size_type i = 0; i < trans2number.size (); i++)
  {
      trans2number[i].resize (nStates, 0);
  }

  number2trans.resize (nTransitions + 1);

  assert (nStates == idx2name.size ());
  assert (nStates == name2idx.size ());

  add_transitions (transitions, nTransitions);
}

void StateMap::add_transitions 
  (const StateTransition transitions[], 
   int nTransitions)
{
  int transId = 1;

  // Loop through all transitions and fill the tables
  for (int i = 0; i < nTransitions; i++)
  {
    // Search 'from' state index by the name
    const Name2Idx::const_iterator fromIdxIt = 
      name2idx.find(transitions[i].from);

    if (fromIdxIt == name2idx.end ())
      throw BadParameters 
      (std::string ("Invalid state name is [")
      + transitions[i].from + ']'
      );

    // Search 'to' state index by the name
    const Name2Idx::const_iterator toIdxIt = 
      name2idx.find(transitions[i].to);

    if (toIdxIt == name2idx.end ())
      throw BadParameters 
      (std::string ("Invalid state name is [")
      + transitions[i].to + ']'
      );

    const StateIdx fromIdx = fromIdxIt->second;
    const StateIdx toIdx = toIdxIt->second;

    // Fill the transition code by two states lookup
    // table.
    if (trans2number.at (fromIdx-1).at (toIdx-1) == 0)
    {
      trans2number.at (fromIdx-1).at (toIdx-1) = transId;
      number2trans.at (transId).from = fromIdx;
      number2trans.at (transId).to = toIdx;
      transId++;
    }
    else
      LOG4CXX_WARN 
        (log.GetLogger (),
        std::string ("Transition ")
        + transitions[i].from
        + " -> "
        + transitions[i].to
        + " is registered already."
        );
  }
}

UniversalState StateMap::create_state 
  (const char* name) const
{
  assert (name);

  Name2Idx::const_iterator cit = name2idx.find (name);
  if (cit != name2idx.end ()) {
    return UniversalState (this, cit->second);
  }
  else 
    throw NoStateWithTheName ();
}

unsigned int StateMap::size () const
{
  return name2idx.size ();
}

inline bool StateMap::there_is_transition 
  (const UniversalState& from,
   const UniversalState& to) const
{
  if (!is_compatible (from) || !is_compatible (to))
    throw IncompatibleMap ();

  return get_transition_id (from, to) != 0;
}

inline int StateMap::get_transition_id 
  (const UniversalState& from,
   const UniversalState& to) const
{
  assert (from.state_idx > 0);
  assert (from.state_idx <= size ());
  assert (to.state_idx > 0);
  assert (to.state_idx <= size ());

  return trans2number.at (from.state_idx-1).at (to.state_idx-1);
}

void StateMap::check_transition
  (const UniversalState& from,
   const UniversalState& to) const
{
  if (!there_is_transition (from, to))
    throw InvalidStateTransition 
      (get_state_name (from),
       get_state_name (to)
       );
}

bool StateMap::is_compatible 
  (const UniversalState& state) const
{
  assert (state.state_map);
  assert (state.state_idx > 0);
  assert (state.state_idx <= size ());

  return state.state_map == this;
  //for inherited carts some code should be added
}

std::string StateMap::get_state_name
  (const UniversalState& state) const
{
  if (!is_compatible (state))
    throw IncompatibleMap ();

  assert (state.state_idx >= 1);
  Idx2Name::const_iterator it = idx2name.find 
    (state.state_idx);
  return it->second;
}

void StateMap::outString (std::ostream& out) const
{
  // print states
  bool first = true;
  for (Idx2Name::const_iterator it = idx2name.begin ();
       it != idx2name.end ();
       it++
       )
  {
      if (!first) 
        out << ';';
      else
        first = false;
  
      out << it->second;
  }
  out << '|';

  //print transitions
  first = true;
  for (Number2Trans::size_type i = 1; i <= number2trans.size (); i++)
  {
      if (!first) 
        out << ';';
      else
        first = false;
  
      out << idx2name.find (number2trans[i].from) -> second
          << "->"
          << idx2name.find (number2trans[i].to) -> second;
  }
}

