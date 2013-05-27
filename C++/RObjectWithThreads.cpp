// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "RObjectWithThreads.h"
#include "RThreadRepository.h"
#include "RState.hpp"
#include "REvent.hpp"
#include <memory>

DEFINE_AXIS(
  ConstructibleAxis,
  { "in_construction", "complete_construction" },
  { { "in_construction", "complete_construction" }
  });

DEFINE_STATES(ConstructibleAxis);
DEFINE_STATE_CONST(RConstructibleObject, 
                   ConstructibleState, 
                   complete_construction);
DEFINE_STATE_CONST(RConstructibleObject, 
                   ConstructibleState, 
                   in_construction);

DEFINE_AXIS(ObjectWithThreadsAxis, {}, {});


RConstructibleObject::RConstructibleObject()
  : RObjectWithEvents<ConstructibleAxis>
    (in_constructionState),
    CONSTRUCT_EVENT(complete_construction)
{
}






