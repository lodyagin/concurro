#include "StdAfx.h"
#include "SShutdown.h"

#ifdef _WIN32
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
#endif

#if 1
// XShuttingDown  ====================================================

XShuttingDown::XShuttingDown( const std::wstring & act ) :
  Parent(sFormat(L"%s action interrupted by shutdown signal", act.c_str())),
  _action(act)
{
}


//====================================================================

void xShuttingDown( const std::wstring & act )
{
  throw XShuttingDown(act);
}
#endif

#ifdef _WIN32
void sCheckShuttingDown()
{
  if ( SSHUTDOWN.isShuttingDown() ) xShuttingDown();
}
#endif
