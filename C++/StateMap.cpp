// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "StateMap.h"
#include <assert.h>
#include <cstdatomic>

// a global
//static std::atomic<TransitionId> max_transition_id = 0;

StateMap* StateMapParBase::create_derivation
  (const ObjectCreationInfo& oi) const
{
  return new StateMap(oi, *this);
}

StateMap* EmptyStateMapPar::create_derivation
  (const ObjectCreationInfo& oi) const
{
  return new EmptyStateMap();
}

StateMap* StateAxis::get_state_map()
{
  return &EmptyStateMap::instance();
}

/*
std::ostream&
operator<< (std::ostream& out, const StateMapId& id)
{
  out << '(' << id.first << ", " << id.second << ')';
  return out;
}
*/

std::ostream& operator<< 
(std::ostream& out, const StateMapParBase& par)
{
  bool first_state = true;
  for (auto it = par.states.begin(); it != par.states.end(); it++)
  {
	 if (!first_state) out << ';'; else first_state = false;
	 out << *it;
  }
  out << '|';
  bool first_trans = true;
  for (auto it = par.transitions.begin(); 
		 it != par.transitions.end(); it++)
  {
	 if (!first_trans) out << ';'; else first_trans = false;
	 out << it->first << "->" << it->second;
  }
  return out;
}

StateMap::StateMap()
  : n_states(0),
	 universal_object_id("<empty map>")
{
}

StateMap::StateMap(const ObjectCreationInfo& oi,
						 const StateMapParBase& par)
  : n_states(par.states.size()
#ifdef PARENT_MAP
				 + parent->get_n_states()
#endif
				 ),
	 universal_object_id(oi.objectId),
    idx2name(n_states+1),
	 name2idx(n_states * 2 + 2),
	 transitions(boost::extents
					 [Transitions::extent_range(1,n_states+1)]
					 [Transitions::extent_range(1,n_states+1)]),
	 //max_transition_id(0),
	 repo(dynamic_cast<StateMapRepository*>(oi.repository))
{
#ifdef PARENT_MAP
  const StateIdx n_parent_states = parent->get_n_states();

  if (parent) {
	 LOG_DEBUG(log, "Create a new map with the parent: "
				  << *parent << "and new_states: "
				  << new_states);
  }
  else {
#endif
	 LOG_DEBUG(log, "Create a new top-level map with states: "
				  << par);
#ifdef PARENT_MAP
  }
#endif

  // Merge the maps: copy old states first
  idx2name[0] = NULL;
#ifdef PARENT_MAP
  std::copy(parent->idx2name.begin(), 
				parent->idx2name.end(),
				idx2name.begin() + 1);
#endif
  // append new states
  std::copy(par.states.begin(), 
				par.states.end(),
				idx2name.begin() + 1 
#ifdef PARENT_MAP
				+ n_parent_states
#endif
);

  // Fill name2idx and check repetitions
  for (StateIdx k = 0; k < idx2name.size(); k++)
  {
	 const auto inserted = name2idx.insert
		(std::pair<const char*, StateIdx>(idx2name[k], k));
	 if (!inserted.second)
		throw BadParameters("map states repetition");
  }

  LOG_DEBUG(log, 
    "There are " << get_n_states() 
				<< " states in the map.");

  // initialize the transitions array
  std::fill(transitions.data(), 
				transitions.data() + transitions.size(), 
				0);
#ifdef PARENT_MAP
  if (!parent->is_empty_map()) {
	 // fill parent (nested) transitions 
	 typedef Transitions::index_range range;
	 Transitions::array_view<2>::type parent_transitions =
		transitions[boost::indices[range(1,n_parent_states)]
						[range(1,n_parent_states)]];
	 parent_transitions = parent->transitions;
	 max_transition_id = parent->max_transition_id();
  }
#endif
  // add new transitions
  // TODO add range checking and exception
  // (now it asserts)
  for (auto tit = par.transitions.begin();
		 tit != par.transitions.end(); tit++)
  {
	 TransitionId& old = transitions
		[name2idx[tit->first]][name2idx[tit->second]];

	 if (!old)
		// count only new transitions
		old = ++(repo->max_trans_id);
  }
}

/*
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
      LOG_WARN (log,
		  "Transition " << transitions[i].from
		  << " -> " << transitions[i].to
		  << " is registered already."
        );
  }
}
*/

#if 1
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
#endif

unsigned int StateMap::size () const
{
  return name2idx.size ();
}

inline bool StateMap::there_is_transition 
  (const UniversalState& from,
   const UniversalState& to) const
{
  return get_transition_id(from, to) != 0;
}

TransitionId StateMap::get_transition_id 
  (const char* from, const char* to) const
{
  return get_transition_id
	 (create_state(from), create_state(to));
}

TransitionId StateMap::get_transition_id 
  (const UniversalState& from,
   const UniversalState& to) const
{
  if (!is_compatible (from) || !is_compatible (to))
    throw IncompatibleMap ();

  return transitions[from.state_idx][to.state_idx];
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

bool StateMap::is_equal
  (const UniversalState& a,
   const UniversalState& b) const
{
  if (!is_compatible (a) || !is_compatible (b))
    throw IncompatibleMap ();

  assert (a.state_idx >= 1);
  assert (b.state_idx >= 1);

  return a.state_idx == b.state_idx;
} 

bool StateMap::is_compatible 
  (const UniversalState& state) const
{
  assert (state.state_map);
  assert (state.state_idx > 0);
  assert (state.state_idx <= size ());

  return state.state_map == this;
  //FIXME for inherited carts some code should be added
}

std::string StateMap::get_state_name
  (const UniversalState& state) const
{
  if (!is_compatible (state))
    throw IncompatibleMap ();

  assert (state.state_idx >= 1);
  return idx2name.at(state.state_idx);
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
  
      out << *it;
  }
  out << '|';

#if 0 //FIXME
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
#endif
}

bool UniversalState::operator== (const UniversalState& st) const
{
	if (this->state_map != st.state_map)
		THROW_EXCEPTION(SException, 
		 "concurro: Comparison of states "
		 "from different maps is not implemented");

	return this->state_idx == st.state_idx;
}

std::string UniversalState::name() const
{
  return state_map->get_state_name(*this);
}


/*StateAxis::StateAxis(StateMap* map_extension, const char* initial_state)
  : UniversalState(map_extension->create_state(initial_state))
{
}*/
