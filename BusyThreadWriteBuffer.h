#pragma once

#include "SMutex.h"
#include "SEvent.h"
#include <algorithm> // for swap

template <class Buffer>
class BusyThreadWriteBuffer
{
public:
  BusyThreadWriteBuffer(void);
  virtual ~BusyThreadWriteBuffer(void);

  // For call from a busy thread
  void put (void* data, size_t len);

  // another thread (a worker)
  // It can wait until data is arrived.
  void* get (size_t* lenp);
protected:
  void swap ();

  Buffer* readBuf;
  Buffer* writeBuf;
  int nWriteBufMsgs; // atomic
  int nReadBufMsgs; // atomic
  SMutex swapM; // a swap guard
  SEvent dataArrived; // nWriteBufMsgs 0->1
};

template<class Buffer>
BusyThreadWriteBuffer<Buffer>::BusyThreadWriteBuffer(void)
: readBuf (0), writeBuf (0), nWriteBufMsgs (0), nReadBufMsgs (0),
  dataArrived (false, false) // automatic reset, initial state = false 
{
  readBuf = new Buffer;
  writeBuf = new Buffer; //FIXME check alloc

  buffer_init (readBuf);
  buffer_init (writeBuf);
}

template<class Buffer>
BusyThreadWriteBuffer<Buffer>::~BusyThreadWriteBuffer(void)
{
  buffer_free (readBuf);
  buffer_free (writeBuf);

  delete readBuf;
  delete writeBuf;
}

template<class Buffer>
void BusyThreadWriteBuffer<Buffer>::put (void* data, size_t len)
{
  SMutex::Lock lock (swapM); // disable buffer swapping

  const bool wasEmpty = nWriteBufMsgs == 0;

  logit ("busy: write string");
  buffer_put_string (writeBuf, data, len);
  // put data with a length marker

  nWriteBufMsgs++;
  logit ("busy: increment writes, now %d", (int) nWriteBufMsgs);
  if (wasEmpty) 
  {
    logit ("busy: signal arriving");
    dataArrived.set ();
  }
}

template<class Buffer>
void* BusyThreadWriteBuffer<Buffer>::get (size_t* lenp)
{ // can work free with the read buffer
  logit("worker: %d messages for me", (int) nReadBufMsgs);
  if (nReadBufMsgs) 
  {
    nReadBufMsgs--;
    logit("worker: get one, now %d", (int) nReadBufMsgs);
    return buffer_get_string (readBuf, lenp);
  }

  dataArrived.reset (); //FIXME remove it
  logit("worker: %d messages on writer side", (int) nWriteBufMsgs);
  if (!nWriteBufMsgs) 
  {
    logit("worker: wait for messages on writer side");
    dataArrived.wait ();
    logit("worker: got it, now %d", (int) nWriteBufMsgs);
  }
  logit("worker: make swap");
  swap ();
  logit("worker: %d messages for me (after swap)", (int) nReadBufMsgs);
  nReadBufMsgs--;
  return buffer_get_string (readBuf, lenp);
}

template<class Buffer>
void BusyThreadWriteBuffer<Buffer>::swap ()
{
  SMutex::Lock lock (swapM);

  // swap read and write buffers
  std::swap (readBuf, writeBuf);
  std::swap (nReadBufMsgs, nWriteBufMsgs);
}
