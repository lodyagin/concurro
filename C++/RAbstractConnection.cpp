#include "RAbstractConnection.h"

const StateMapPar RAbstractConnection::new_states
({"closed", "established", "destroyed"},
 {
	{"closed", "established"},
	{"established", "closed"},
	{"closed", "destroyed"}
 });

const RAbstractConnection::State RAbstractConnection::closedState("closed");
const RAbstractConnection::State RAbstractConnection::establishedState("established");
const RAbstractConnection::State RAbstractConnection::destroyedState("destroyed");
