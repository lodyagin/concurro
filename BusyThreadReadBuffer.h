#pragma once

#include "SMutex.h"
#include "SEvent.h"
#include <algorithm> // for swap

template<class Buffer>
class BusyThreadReadBuffer
{
public:
  BusyThreadReadBuffer(void);
  virtual ~BusyThreadReadBuffer(void);
  
  // For call from a worker
  void put (void* data, size_t len);

  // For call from a busy thread
  // Return 0 if no data
  bool get (Buffer* out);

  SEvent dataReady;
protected:
  void swap ();

  Buffer* readBuf;
  Buffer* writeBuf;
  int nWriteBufMsgs; // atomic
  int nReadBufMsgs; // atomic
  SMutex swapM; // a swap guard
};

template<class Buffer>
BusyThreadReadBuffer<Buffer>::BusyThreadReadBuffer(void)
: readBuf (0), writeBuf (0), nWriteBufMsgs (0), nReadBufMsgs (0),
  dataReady (true, false) // manual reset, initial state =  non signalled
{
  readBuf = new Buffer;
  writeBuf = new Buffer; //FIXME check alloc

  buffer_init (readBuf);
  buffer_init (writeBuf);
}

template<class Buffer>
BusyThreadReadBuffer<Buffer>::~BusyThreadReadBuffer(void)
{
  buffer_free (readBuf);
  buffer_free (writeBuf);

  delete readBuf;
  delete writeBuf;
}

template<class Buffer>
void BusyThreadReadBuffer<Buffer>::put (void* data, size_t len)
{ // can work free with the write buffer
  logit("worker2: put message");
  buffer_put_string (writeBuf, data, len);
  // put data with a length marker
  nWriteBufMsgs++;
  logit("worker2: now %d messages on my side"
    " and %d on a reader side", (int) nWriteBufMsgs, (int) nReadBufMsgs);

  if (!nReadBufMsgs) 
  {
    logit("worker2: make swap");
    swap ();
    logit("worker2: %d messages for me (after swap)", (int) nWriteBufMsgs);
    logit("worker2: set data ready");
    dataReady.set ();
  }
}

template<class Buffer>
bool BusyThreadReadBuffer<Buffer>::get (Buffer* out)
{
  SMutex::Lock lock (swapM);

  logit("busy2: %d messages for me", (int) nReadBufMsgs);

  if (nReadBufMsgs) 
  {
    u_int lenp;
    nReadBufMsgs--;
    void* data = buffer_get_string (readBuf, &lenp);
    buffer_put_string (out, data, lenp);
    logit("busy2: get one, now %d", (int) nReadBufMsgs);
    
    if (!nReadBufMsgs)
    {
      logit("busy2: reset data ready");
      dataReady.reset ();
    }
    
    return true;
  }
  else
  {
    logit("busy2: reset data ready");
    dataReady.reset ();
    return false;
  }
}

template<class Buffer>
void BusyThreadReadBuffer<Buffer>::swap ()
{
  SMutex::Lock lock (swapM);

  // swap read and write buffers
  std::swap (readBuf, writeBuf);
  std::swap (nReadBufMsgs, nWriteBufMsgs);
}
