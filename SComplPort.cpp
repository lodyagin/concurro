#include "stdafx.h"
#include "SComplPort.h"
#include "SShutdown.h"


SComplPort::SComplPort( size_t _threadCount ) :
  h(CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, DWORD(_threadCount))),
  threadCount(_threadCount)
{
  sWinCheck(h != 0, "creating I/O completion port");
}

SComplPort::~SComplPort()
{
  if ( h ) CloseHandle(h);
}

void SComplPort::assoc( HANDLE file, size_t key )
{
  sWinCheck(CreateIoCompletionPort(file, h, key, DWORD(threadCount)) != 0,
    "associating I/O completion port with a file");
}

bool SComplPort::getStatus( size_t & transferred, size_t & key, OVERLAPPED *& ov )
{
  DWORD _transferred = 0;
  ULONG_PTR _key = 0;
  SSHUTDOWN.registerComplPort(*this);
  BOOL result = GetQueuedCompletionStatus(h, &_transferred, &_key, &ov, INFINITE);
  SSHUTDOWN.unregisterComplPort(*this);
  if ( result ) 
  {
    sCheckShuttingDown();

    transferred = _transferred;
    key = _key;
    return true;
  }
  sWinCheck(ov != 0, "getting queued completion status");
  return false;
}

void SComplPort::postEmptyEvt()
{
  sWinCheck(PostQueuedCompletionStatus(h, 0, 0, 0), 
    "posting empty queued completion status");
}
