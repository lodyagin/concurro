#include "StdAfx.h"
#include "RSocket.h"

// RSocket states  ========================================

const State2Idx RThreadBase::allStates[] =
{
  {1, "initialized"},  // after creation
  {2, "listen"},       // passive open
  {3, "terminated"},    
  {4, "destroyed"},    // to check a state in the destructor
  {0, 0}
};

const StateTransition RThreadBase::allTrans[] =
{
  {"ready", "working"},      // start ()

  // a) natural termination
  // b) termination by request
  {"working", "terminated"},      

  {"terminated", "destroyed"},

  {"ready", "destroyed"},
  // can't be destroyed in other states

  {0, 0}

};

RThreadBase::ThreadState RThreadBase::readyState("ready");
RThreadBase::ThreadState RThreadBase::workingState("working");
RThreadBase::ThreadState RThreadBase::terminatedState("terminated");
RThreadBase::ThreadState RThreadBase::destroyedState("destroyed");



