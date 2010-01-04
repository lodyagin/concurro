#pragma once
#include "smutex.h"
#include "snotcopyable.h"
#include "sexception.h"
#include <set>

template<class Object, class Parameter>
class Repository : public SNotCopyable
{
public:
  class UnregisteredObject : public SException
  {
  public:
    UnregisteredObject ()
      : SException ("Unregistered object")
    {}
  };

  Repository (int maxNumberOfObjects);
  ~Repository ();

  virtual Object* create_object (Parameter param);
  virtual void delete_object (Object* obj, bool freeMemory);

protected:
  typedef std::set<Object*> ObjectSet;

  ObjectSet* objects;
  SMutex objectsM;
  SSemaphore semaphore;
};

template<class Object, class Parameter>
Repository<Object, Parameter>::Repository 
  (int maxNumberOfObjects)
: objects (NULL), 
  semaphore (maxNumberOfObjects, maxNumberOfObjects)
{
  objects = new ObjectSet ();
  //FIXME check obj creation
}

template<class Object, class Parameter>
Repository<Object, Parameter>::~Repository ()
{
  //FIXME add some code...
  delete objects;
}

template<class Object, class Parameter>
Object* Repository<Object, Parameter>::create_object 
  (Parameter param)
{
  semaphore.wait ();

  if (SThread::current ().is_stop_requested ())
       ::xShuttingDown 
        ("Stop request from the owner thread.");


  Object* obj = Object::create (this, param);
  //FIXME check creation

  { 
    SMutex::Lock lock (objectsM, true, true);
    objects->insert (obj);
  }
  return obj;
}

template<class Object, class Parameter>
void Repository<Object, Parameter>::delete_object 
  (Object* obj, 
   bool freeMemory
   )
{
  {
    SMutex::Lock lock (objectsM, true, true);

    if (objects->erase (obj) != 1)
      throw UnregisteredObject ();
  }

  semaphore.release ();

  if (freeMemory)
    THROW_EXCEPTION
      (SException, oss_ << "Not implemented");
}




