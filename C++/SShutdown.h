#ifndef __SSHUTDOWN_H
#define __SSHUTDOWN_H

#ifdef _WIN32
#include "SComplPort.h"
#endif
#include "SSingleton.h"
#include "SException.h"
#include "SCommon.h"
#include <vector>


#ifdef _WIN32
#define SSHUTDOWN  SShutdown::instance()


class SShutdown : public SSingleton<SShutdown>
{
public:

  SShutdown();
  ~SShutdown();

  void shutdown();  // shutdown application (or current thread)
  bool isShuttingDown();

  HANDLE event() { return evt; }

  void registerComplPort( SComplPort & );
  void unregisterComplPort( SComplPort & );

private:

  friend class SSocket;

  HANDLE evt;
  std::vector<SComplPort *> ports;

};
#endif

#if 1
class XShuttingDown : public SException
{
public:

  typedef SException Parent;

  explicit XShuttingDown( const std::wstring & interruptedAction = L"unknown" );
  ~XShuttingDown () throw () {};

private:

  std::wstring _action;

};


// throw XShuttindDown
void xShuttingDown( const std::wstring & interruptedAction = L"unknown" );
void sCheckShuttingDown();  // throws ZSD if is shuttind down
#endif

#endif  // __SSHUTDOWN_H
