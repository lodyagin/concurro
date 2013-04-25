// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * An unified wrapper over different type of threads (i.e., QThread, posix thread etc.).
 */

#ifndef CONCURRO_RTHREAD_HPP_
#define CONCURRO_RTHREAD_HPP_

template<class Thread, class... Args>
Thread* RThread<std::thread>::create(Args&&... args)
{
  return dynamic_cast<Thread*>
	 (RThreadRepository<RThread<std::thread>>::instance()
	  . create_thread(typename Thread::Par(args...)));
}

#endif

