#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <string>
#include "SEvent.h"
#include "SMutex.h"

class NamedPipe
{
public:
  enum Mode { Read, Write, Duplex };

  NamedPipe 
    (const std::wstring& _name,
     Mode _mode
     );

  virtual ~NamedPipe ();

  HANDLE get_server_hanle () const
  {
    return serverPart;
  }

  HANDLE get_client_handle () const
  {
    return clientPart;
  }

  void StartRead (void* buf, DWORD bufSize);
  void CompleteRead (LPDWORD nBytesRed);

  bool read_started () const
  { return readStarted; }

  const Mode mode;
  const std::wstring name;

  SEvent readingIsAvailable;

protected:
  enum { PipeInBufSize = 80 };
  enum { PipeOutBufSize = 80 };

  HANDLE serverPart;
  HANDLE clientPart;

  OVERLAPPED readOverlap;
  SMutex readInProgress;
  bool readStarted;
};
