#include "RAbstractConnection.h"

const State2Idx RAbstractConnection::allStates[] =
{
  {1, "closed"},  
  {2, "established"},
  {3, "destroyed"}, // to check a state in the destructor
  {0, 0}
};

const StateTransition RAbstractConnection::allTrans[] =
{
  {"closed", "established"},
  {"established", "closed"},
  {"closed", "destroyed"},
  {0, 0}
};


const RAbstractConnection::State RAbstractConnection::closedState("closed");
const RAbstractConnection::State RAbstractConnection::establishedState("established");
const RAbstractConnection::State RAbstractConnection::destroyedState("destroyed");
