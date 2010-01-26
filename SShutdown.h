#ifndef __SSHUTDOWN_H
#define __SSHUTDOWN_H

#include "SComplPort.h"
#include "SSingleton.h"
#include "SException.h"
#include "SCommon.h"
#include <vector>


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


class XShuttingDown : public SException
{
public:

  typedef SException Parent;

  explicit XShuttingDown( const wstring & interruptedAction = L"unknown" );

private:

  wstring _action;

};


// throw XShuttindDown
void xShuttingDown( const wstring & interruptedAction = L"unknown" );
void sCheckShuttingDown();  // throws ZSD if is shuttind down


#endif  // __SSHUTDOWN_H
