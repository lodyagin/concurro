// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "StateMap.h"
#include "RState.h"
#include <assert.h>
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif

std::string UniversalState::name() const
{
  return StateMapRepository::instance()
	 . get_state_name(the_state);
}

#if 0
bool UniversalState::operator== 
  (uint32_t st) const
{
	if (this->state_map != st.state_map)
		THROW_EXCEPTION(SException, 
		 "concurro: Comparison of states "
		 "from different maps is not implemented");

	return this->state_idx == st.state_idx;
}

bool UniversalState::operator!= 
  (uint32_t st) const
{
	if (this->state_map != st.state_map)
		THROW_EXCEPTION(SException, 
		 "concurro: Comparison of states "
		 "from different maps is not implemented");

	return this->state_idx != st.state_idx;
}
#endif

std::ostream&
operator<< (std::ostream& out, const UniversalState& st)
{
  out << st.name();
  return out;
}

UniversalEvent::operator UniversalState() const
{
  if (!is_arrival_event())
	 throw NeedArrivalType();
  return UniversalState(STATE_IDX(id));
}

std::string UniversalEvent::name() const
{
  if (is_arrival_event())
	 return UniversalState(as_arrival()).name();
  else
	 return "<todo>";
}

std::ostream&
operator<< (std::ostream& out, const UniversalEvent& ue)
{
  out << ue.name();
  return out;
}

InvalidState::InvalidState(UniversalState current,
									UniversalState expected)
  : SException
	 (SFORMAT("Invalid state [" << current
				 << "] when expected [" << expected << "]"))
  {}

NoStateWithTheName::NoStateWithTheName
  (const std::string& name, 
	const StateMap* map)
  : 
	 SException (SFORMAT(
				 "No state with the name [" << name << "]"
				 << " in the map " << (*map)))
{}

StateMap* StateMapParBase::create_derivation
  (const ObjectCreationInfo& oi) const
{
  return new StateMap(oi, *this);
}

StateMapId StateMapParBase::get_map_id
(const ObjectCreationInfo& oi,
 const std::type_info& axis) const
{
  return dynamic_cast<StateMapRepository*>
	 (oi.repository)->get_map_id(axis);
}


std::ostream& operator<< 
(std::ostream& out, const StateMapParBase& par)
{
  bool first_state = true;
  for (auto it = par.states.begin(); 
		 it != par.states.end(); it++)
  {
	 if (!first_state) out << ';'; 
	 else first_state = false;
	 out << *it;
  }
  out << '|';
  bool first_trans = true;
  for (auto it = par.transitions.begin(); 
		 it != par.transitions.end(); it++)
  {
	 if (!first_trans) out << ';'; 
	 else first_trans = false;
	 out << it->first << "->" << it->second;
  }
  return out;
}

StateMap::StateMap()
  : 
    universal_object_id("0"),
	 numeric_id(0),
	 parent(nullptr),
    n_states(0),
	 idx2name(1),
	 repo(&StateMapRepository::instance())
{
  idx2name[0] = std::string();
}

StateMap::StateMap(const ObjectCreationInfo& oi,
						 const StateMapParBase& par)
  : 
	 universal_object_id(oi.objectId),
	 numeric_id(fromString<int16_t>(oi.objectId)),
	 parent(StateMapRepository::instance()
			  . get_object_by_id(par.parent_map)),
    n_states(par.states.size()
				 + parent->get_n_states()
				 ),
	 name2idx(n_states * 2 + 2),
    idx2name(n_states+1),
	 transitions(boost::extents
					 [Transitions::extent_range(1,n_states+1)]
					 [Transitions::extent_range(1,n_states+1)]),
	 repo(dynamic_cast<StateMapRepository*>(oi.repository))
{
  // TODO check numeric_id overflow

  const StateIdx n_parent_states = parent->get_n_states();

  if (parent) {
	 LOG_DEBUG(log, "Create a new map with the parent: "
				  << *parent << "and new_states: " << par);
  }
  else {
	 LOG_DEBUG(log, 
				  "Create a new top-level map with states: "
				  << par);
  }

  // Merge the maps: copy old states first
  idx2name[0] = std::string();
  std::copy(parent->idx2name.begin() + 1, 
				parent->idx2name.end(),
				idx2name.begin() + 1);
  // append new states
  std::copy(par.states.begin(), 
				par.states.end(),
				idx2name.begin() + 1 
				+ n_parent_states
);

  // Fill name2idx and check repetitions
  for (StateIdx k = 0; k < idx2name.size(); k++)
  {
	 const auto inserted = name2idx.insert
		(std::pair<std::string, StateIdx>(idx2name[k], k));
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
  if (!parent->is_empty_map()) {
	 // fill parent (nested) transitions 
	 typedef Transitions::index_range range;
	 Transitions::array_view<2>::type parent_transitions =
		transitions[boost::indices
						[range(1,n_parent_states+1)]
						[range(1,n_parent_states+1)]];
	 parent_transitions = parent->transitions;
  }
  // add new transitions
  // TODO add range checking and exception
  // (now it asserts)
  for (auto tit = par.transitions.begin();
		 tit != par.transitions.end(); tit++)
  {
	 const StateIdx st1 = name2idx.at(tit->first);
	 const StateIdx st2 = name2idx.at(tit->second);
	 TransitionId& old = transitions[st1][st2];

	 if (!old) {
		// count only new transitions
		old = ++(repo->max_trans_id);
		trans2states[old] = std::pair<StateIdx, StateIdx>
		  (st1, st2);
	 }
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

uint32_t StateMap::create_state (const char* name) const
{
  assert (name);

  Name2Idx::const_iterator cit = name2idx.find (name);
  if (cit != name2idx.end ()) {
    return (numeric_id << STATE_MAP_SHIFT) 
		| STATE_IDX(cit->second);
  }
  else 
    throw NoStateWithTheName(name, this);
}

StateIdx StateMap::size () const
{
  return name2idx.size ();
}

inline bool StateMap::there_is_transition 
  (uint32_t from,
   uint32_t to) const
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
  (uint32_t from,
   uint32_t to) const
{
  if (!is_compatible (from) || !is_compatible (to))
    throw IncompatibleMap ();

  const StateIdx from_idx = STATE_IDX(from);
  const StateIdx to_idx = STATE_IDX(to);

  return transitions[from_idx][to_idx];
}

void StateMap::get_states(TransitionId trans_id,
								  uint32_t& from, uint32_t& to)
{
  const std::pair<StateIdx, StateIdx>& t =
	 trans2states.at(trans_id);
  from = t.first;
  to = t.second;
}

void StateMap::check_transition
  (uint32_t from,
   uint32_t to) const
{
  if (!there_is_transition (from, to))
    throw InvalidStateTransition(from, to);
}

bool StateMap::is_compatible(uint32_t state) const
{
  return STATE_MAP(state) == (uint32_t) numeric_id;
}

std::string StateMap::get_state_name
  (uint32_t state) const
{
  if (!is_compatible (state))
    throw IncompatibleMap ();

  return idx2name.at(STATE_IDX(state));
}

void StateMap::outString (std::ostream& out) const
{
  // print states
  bool first = true;
  for (Idx2Name::const_iterator it 
			= idx2name.begin() + 1; //skip a "null" state
       it != idx2name.end();
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

StateMap* StateMapRepository::empty_map = nullptr;

StateMap* StateMapRepository
::get_object_by_id(StateMapId id) const
{
  if (id != 0)
	 return Parent::get_object_by_id(id);
  else {
	 if (!empty_map)
		empty_map = new StateMap();
	 return empty_map;
  }
}

StateMap* StateMapRepository::get_map_for_axis
  (const std::type_info& axis)
{
  RLOCK(objectsM);
  return get_object_by_id(get_map_id(axis));
}

std::string StateMapRepository
//
::get_state_name(uint32_t state) const
{
  return get_object_by_id(STATE_MAP(state))
	 -> get_state_name(state);
}

StateMapId StateMapRepository::get_map_id
  (const std::type_info& axis)
{
  if (axis == typeid(StateAxis))
	 return 0; // empty map for the abstract axis

  const std::string& name = axis.name();
  auto it = axis2map_id.find(name);
  if (it == axis2map_id.end()) {
	 axis2map_id.insert
		(std::pair<std::string, StateMapId>
		 (name, ++last_map_id));
	 return last_map_id;
  }
  else return it->second;
}


