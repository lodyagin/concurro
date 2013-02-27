// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef SSINGLETON_H_
#define SSINGLETON_H_

#include "SCheck.h"

class DeadReferenceException : public std::exception{};

/**
 * Base class for classes that can have only one instance
 * parametrised by the actual singleton class use as:
 * class MyClass : public SSingleton<MyClass>
 */
template<class T>
class SSingleton
{
public:
  /// One and only one class instance must be created with
  /// this function.
  SSingleton();
  virtual ~SSingleton();

  /// Return the reference to the class instance. If no
  /// class is crated with SSingleton() raise exception.
  static T & instance();

  // Added by slod to prevent assertion with MainWin in
  // Common::ErrorMessage.
  // It is not true singleton (why?) and thus we need a
  // trick.
  static bool isConstructed ()
  {
     return _instance != NULL;
  }

private:

  static T * _instance;
  static bool destroyed;

};

/**
 * Extends SSingleton to allow auto-construct it by the
 * RAutoSingleton::instance() call.
 */
template<class T>
class SAutoSingleton : public SSingleton<T>
{
public:
  static T & instance () {
	 if (!SSingleton<T>::isConstructed ()) new T (); 
	 return SSingleton<T>::instance ();
  }
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
  SCHECK(_instance);
  _instance = 0;
  destroyed = true;
}

template<class T>
inline T & SSingleton<T>::instance()
{
  if(destroyed)
  {
     throw DeadReferenceException();
  }
   SPRECONDITION(_instance);
  return *_instance;
}

// strange construction
template<class T>
T * SSingleton<T>::_instance = 0;
template<class T> bool SSingleton<T>::destroyed = false;



#endif
