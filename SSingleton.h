#ifndef __SSINGLETON_H
#define __SSINGLETON_H

#include "SCheck.h"


// base class for classes that can have only one instance
// parametrized by the actual singleton class
// use as: class MyClass : public SSingleton<MyClass>

class DeadReferenceExeption : public std::exception{};

template<class T>
class SSingleton
{
public:
 
  SSingleton();
  virtual ~SSingleton();
// ?attention: destructor is not virtual! Never do "delete singleton"

  static T & instance();

  // Added by slod to prevent assertion with MainWin in Common::ErrorMessage.
  // It is not true singleton (why?) and thus we need a trick.
  static bool isConstructed ()
  {
     return _instance != NULL;
  }

private:

  static T * _instance;
  static bool destroyed;

};


template<class T>
SSingleton<T>::SSingleton()
{
  SPRECONDITION(!_instance);
  destroyed = false;
  _instance = static_cast<T *>(this);
}

template<class T>
SSingleton<T>::~SSingleton()
{
  SWARN(!_instance, "singleton dtr without ctr?!");
  _instance = 0;
  destroyed = true;
}

template<class T>
inline T & SSingleton<T>::instance()
{
  if(destroyed)
  {
     throw DeadReferenceExeption();
  }
   SPRECONDITION(_instance);
  return *_instance;
}

// strange construction
template<class T>
T * SSingleton<T>::_instance = 0;
template<class T> bool SSingleton<T>::destroyed = false;


#endif  // __SSINGLETON_H
