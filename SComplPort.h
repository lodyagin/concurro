#ifndef __SCOMPLPORT_H
#define __SCOMPLPORT_H

#include "SCommon.h"


// windows I/O completion port wrapper
class SComplPort
{
public:

  explicit SComplPort( size_t threadCount );
  ~SComplPort();

  void assoc( HANDLE, size_t key );
  
  // false if queued operation completed with error - call GetLastError for returned key/ov
  bool getStatus( size_t & transferred, size_t & key, OVERLAPPED *& );

  void postEmptyEvt();

private:

  HANDLE h;
  size_t threadCount;

};


#endif  // __SCOMPLPORT_H
