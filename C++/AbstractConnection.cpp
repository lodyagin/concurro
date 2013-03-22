#include "AbstractConnection.h"

const StateMapPar AbstractConnection::new_states
({"closed", "established", "destroyed"}, {});

const AbstractConnection::State 
AbstractConnection::closedState("closed");

const AbstractConnection::State 
AbstractConnection::establishedState("established");

const AbstractConnection::State 
AbstractConnection::destroyedState("destroyed");
