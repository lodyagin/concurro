#pragma once

#include "SMutex.h"
#include "SEvent.h"
#include <algorithm> // for swap

template <class Buffer>
class BusyThreadWriteBuffer
{
public:
  BusyThreadWriteBuffer();
  virtual ~BusyThreadWriteBuffer(void);

  // For call from a busy thread;
  // *consumed is increased by the size of data which gone
  void put (void* data, u_int32_t len, u_int* consumed);

  // another thread (a worker)
  // It can wait until data is arrived.
  void* get (u_int32_t* lenp);
protected:
  void swap ();

  Buffer* readBuf;
  Buffer* writeBuf;
  volatile int nWriteBufMsgs; // atomic
  volatile int nReadBufMsgs; // atomic

  int nWriteConsumed; //atomic, flow control: 
    // consumed by reader (2 sides)
  //FIXME check int overflow
  int nReadConsumed; //atimic 

  SMutex swapM; // a swap guard
  SEvent dataArrived; // nWriteBufMsgs 0->1

  HANDLE events[2];
};

template<class Buffer>
BusyThreadWriteBuffer<Buffer>::BusyThreadWriteBuffer(void)
: readBuf (0), writeBuf (0), nWriteBufMsgs (0), nReadBufMsgs (0),
  nWriteConsumed (0), nReadConsumed (0),
  dataArrived (false, false) // automatic reset, initial state = false 
{
  readBuf = new Buffer;
  writeBuf = new Buffer; //FIXME check alloc

  buffer_init (readBuf);
  buffer_init (writeBuf);

  events[0] = 0;
  events[1] = dataArrived.evt ();
}

template<class Buffer>
BusyThreadWriteBuffer<Buffer>::~BusyThreadWriteBuffer(void)
{
  buffer_free (readBuf);
  buffer_free (writeBuf);

  delete readBuf;
  delete writeBuf;
  //TODO if consumed != 0 here ?
}

template<class Buffer>
void BusyThreadWriteBuffer<Buffer>::put 
  (void* data, u_int32_t len, u_int* consumed)
{
  SMutex::Lock lock (swapM); // disable buffer swapping

  *consumed -= nWriteConsumed; // nWriteConsumed < 0
   // nReadConsumed will be nWriteConsumed after the swap
  //if (*consumed) logit ("busy: %u consumed", (unsigned) *consumed);
  nWriteConsumed = 0;

  if (len == 0) return; // busy getting consume only

  const bool wasEmpty = nWriteBufMsgs == 0;

  //logit ("busy: write string");
  buffer_put_string (writeBuf, data, len);
  // put data with a length marker

  nWriteBufMsgs++;
  //logit ("busy: increment writes, now %d", (int) nWriteBufMsgs);
  if (wasEmpty) 
  {
    //logit ("busy: signal arriving");
    dataArrived.set ();
  }
}

template<class Buffer>
void* BusyThreadWriteBuffer<Buffer>::get (u_int32_t* lenp)
{ // can work free with the read buffer
  void* data = 0;

  if (events[0] == 0) // register this thread to cancel wait on stop
    events[0] = SThread::current ().get_stop_event ().evt ();

  //logit("worker: %d messages for me", (int) nReadBufMsgs);
  if (nReadBufMsgs) 
  {
    nReadBufMsgs--;
    //logit("worker: get one, now %d", (int) nReadBufMsgs);
    data = buffer_get_string (readBuf, lenp);
    nReadConsumed -= * lenp; // only data part 
    //logit ("worker: %d consumed, now %d", (int) *lenp, (int) nReadConsumed);
    return data;
  }

  dataArrived.reset (); //FIXME remove it
  //logit("worker: %d messages on writer side", (int) nWriteBufMsgs);
  if (!nWriteBufMsgs) 
  {
    //logit("worker: swap and wait for messages on writer side");
    swap (); // push consumed

    // wait for events
    if (waitMultiple (events, 2) == 0) // thread stop
    {
      *lenp = 0;
      return 0;
    }

    if (!nWriteBufMsgs) swap (); // miss each other
    //logit("worker: got it, now %d", (int) nWriteBufMsgs);
  }
  //logit("worker: make swap");
  swap ();
  //logit("worker: %d messages for me (after swap)", (int) nReadBufMsgs);
  nReadBufMsgs--;
  data = buffer_get_string (readBuf, lenp);
  nReadConsumed -= * lenp ; 
  //logit ("worker: %d consumed, now %d", (int) *lenp, (int) nReadConsumed);
  return data;  // TODO two identical parts
}

template<class Buffer>
void BusyThreadWriteBuffer<Buffer>::swap ()
{
  SMutex::Lock lock (swapM);

  // swap read and write buffers
  std::swap (readBuf, writeBuf);
  std::swap (nReadBufMsgs, nWriteBufMsgs);
  std::swap (nReadConsumed, nWriteConsumed);
}
