// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSignal.h"

RSignalBase* RSignalBase::Par
::create_derivation(const ObjectCreationInfo& oi) const
{
  switch(par.action) {
  case RSignalAction::Ignore:
	 return new RIgnoredSignal(oi, *this);
  case RSignalAction::Process:
	 return new RSignalHandler(oi, *this);
  default:
	 THROW_NOT_IMPLEMENTED;
  }
}

RSignalBase::RSignalBase
  (const ObjectCreationInfo& oi, const Par& par)
	 : signum(par.signum), action(par.action)
{}

void RSignalRepository::tune_signal
  (int signum, RSignalAction action)
{
  auto* sig = get_object_by_id(signum);
  if (!sig) {
	 sig = create_object(RSignalBase::Par(signum, action));
  }
  else {
	 replace_object(id, RSignalBase::Par(signum, action),
						 true);
  }
}

