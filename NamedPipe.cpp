#include "StdAfx.h"
#include "NamedPipe.h"

NamedPipe::NamedPipe
  (const std::wstring& _name,
   Mode _mode
   )
  : name (_name),
  mode (_mode),
  serverPart (INVALID_HANDLE_VALUE),
  clientPart (INVALID_HANDLE_VALUE),
  readingIsAvailable (true), // manual
  readStarted (false)
{
  ::ZeroMemory 
    (&readOverlap, sizeof (readOverlap));
  readOverlap.hEvent = readingIsAvailable.evt ();
    
  const std::wstring pipeName = 
    L"\\\\.\\pipe\\" + name;

  DWORD serverOpenMode = 
    FILE_FLAG_FIRST_PIPE_INSTANCE
    | FILE_FLAG_OVERLAPPED;
  DWORD clientOpenMode = 0;

  switch (mode) 
  {
  case Read:
    serverOpenMode |= PIPE_ACCESS_INBOUND;
    clientOpenMode = GENERIC_WRITE;
    break;
  case Write:
    serverOpenMode |= PIPE_ACCESS_OUTBOUND;
    clientOpenMode = GENERIC_READ;
    break;
  case Duplex:
    serverOpenMode |= PIPE_ACCESS_DUPLEX;
    clientOpenMode = GENERIC_READ | GENERIC_WRITE;
    break;
  }

  serverPart = ::CreateNamedPipeW
    (pipeName.c_str (),
     serverOpenMode,
     PIPE_TYPE_BYTE | PIPE_READMODE_BYTE
     | PIPE_REJECT_REMOTE_CLIENTS,
     1, // the number of pipe instances
     PipeOutBufSize,
     PipeInBufSize,
     0,
     NULL
     );
  sWinCheck 
    (serverPart != INVALID_HANDLE_VALUE);

  SECURITY_ATTRIBUTES saAttr = {0}; 
  // Set the bInheritHandle flag so 
  // pipe handles are inherited. 
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
  saAttr.bInheritHandle = TRUE; 
  saAttr.lpSecurityDescriptor = NULL; 

  clientPart = ::CreateFileW
    (pipeName.c_str (),
     clientOpenMode,
     0, // no sharing
     &saAttr,
     OPEN_EXISTING,
     FILE_ATTRIBUTE_NORMAL,
     NULL
     );
  sWinCheck 
    (clientPart != INVALID_HANDLE_VALUE);
}

NamedPipe::~NamedPipe(void)
{
  // FIXME !
}

void NamedPipe::StartRead 
  (void* buf, DWORD bufSize)
{
  if (mode == Write)
    THROW_EXCEPTION 
      (SException, 
       L"Impossible to read in Write mode"
       );

  readInProgress.acquare ();
  readStarted = true;
  // No second enter while the read completes

  ::ZeroMemory 
    (&readOverlap, sizeof (readOverlap));
  readOverlap.hEvent = readingIsAvailable.evt ();
    
  const BOOL res = ::ReadFile
      (serverPart,
       buf,
       bufSize,
       NULL,
       &readOverlap
       );

  if (!res && ::GetLastError () != ERROR_IO_PENDING)
    sWinCheck (FALSE);
}

void NamedPipe::CompleteRead (LPDWORD nBytesRed)
{
  if (!readStarted)
    THROW_EXCEPTION
      (SException,
       L"CompleteRead without StartRead"
       );

  sWinCheck
    (::GetOverlappedResult
      (serverPart,
       &readOverlap,
       nBytesRed,
       FALSE)
     );
  readStarted = false;
  //readingIsAvailable.reset ();
  readInProgress.release ();
}
