#include "stdafx.h"
#include "SShutdown.h"


// SShutdown  ========================================================

SShutdown::SShutdown() :
  evt(CreateEvent(0, true, false, 0))
{             // SA manual  init name
  sWinCheck(evt != 0, L"creating event");
}

SShutdown::~SShutdown()
{
  CloseHandle(evt);
}

void SShutdown::shutdown()
{
  sWinCheck(SetEvent(evt), L"Setting event");

  for ( size_t i = 0; i < ports.size(); ++i )
    ports[i]->postEmptyEvt();
}

bool SShutdown::isShuttingDown()
{
  return WaitForSingleObject(evt, 0) == WAIT_OBJECT_0;
}

void SShutdown::registerComplPort( SComplPort & port )
{
  ports.push_back(&port);
}

void SShutdown::unregisterComplPort( SComplPort & port )
{
  for ( size_t i = 0; i < ports.size(); ++i )
    if ( ports[i] == &port ) 
    {
      ports.erase(ports.begin() + i);
      return;
    }
  SCHECK(0);  // unregistering non-registered port
}


// XShuttingDown  ====================================================

XShuttingDown::XShuttingDown( const wstring & act ) :
  Parent(sFormat(L"%s action interrupted by shutdown signal", act.c_str())),
  _action(act)
{
}


//====================================================================

void xShuttingDown( const wstring & act )
{
  throw XShuttingDown(act);
}

void sCheckShuttingDown()
{
  if ( SSHUTDOWN.isShuttingDown() ) xShuttingDown();
}
