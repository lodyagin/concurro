/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_INSOCKET_H_
#define CONCURRO_INSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

class InSocket : virtual public RSocketBase
{
public:
  virtual void ask_close();

  std::string universal_id() const override
  {
    return RSocketBase::universal_id();
  }

  //! The last received data
  RSingleBuffer msg;

protected:
  InSocket (const ObjectCreationInfo& oi, 
            const RSocketAddress& par);

  class SelectThread : public SocketThreadWithPair
  {
  public:
    struct Par : public SocketThreadWithPair::Par
    { 
    Par(RSocketBase* sock) 
      : SocketThreadWithPair::Par(sock) 
      {
        thread_name = SFORMAT("InSocket(select):" 
                              << sock->fd);
      }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new SelectThread(oi, *this); 
      }
    };

    void run();

  protected:
    SelectThread
      (const ObjectCreationInfo& oi, const Par& p)
      : SocketThreadWithPair(oi, p) {}

    ~SelectThread() { destroy(); }
  }* select_thread;

  class WaitThread : public SocketThread
  {
  public:
    struct Par : public SocketThread::Par
    { 
      SOCKET notify_fd;
    Par(RSocketBase* sock, SOCKET notify) 
      : SocketThread::Par(sock),
        notify_fd(notify)
        {
          thread_name = SFORMAT("InSocket(wait):" 
                                << sock->fd);
        }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new WaitThread(oi, *this); 
      }
    };

    void run();

  protected:
    SOCKET notify_fd;

    WaitThread
      (const ObjectCreationInfo& oi, const Par& p)
      : SocketThread(oi, p), notify_fd(p.notify_fd) {}
    ~WaitThread() { destroy(); }
  }* wait_thread;

  //! Actual size of a socket internal read buffer + 1.
  int socket_rd_buf_size;
  SOCKET notify_fd;
  
  ~InSocket();

  DEFAULT_LOGGER(InSocket);
};

//! @}

}
#endif
