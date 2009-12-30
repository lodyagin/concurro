#pragma once
#include "smutex.h"
#include "snotcopyable.h"
#include "sexception.h"
#include <set>

template<class Object, class Parameter>
class Repository : public SNotCopyable
{
public:
  /*class NoMoreObjectsPossible : public SException
  {
  public:
    NoMoreObjectsPossible ()
      : SException ("No more objects possible")
    {}
  };*/

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
  //virtual void register_object (Object* obj);
  virtual void delete_object (Object* obj, bool freeMemory);

protected:
  typedef std::set<Object*> ObjectSet;

  ObjectSet* objects;
  SMutex objectsM;

  const unsigned int nObjects;
};

template<class Object, class Parameter>
Repository<Object, Parameter>::Repository 
  (int maxNumberOfObjects)
: objects (NULL), nObjects (maxNumberOfObjects)
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
  //FIXME wait for object
  while (1)
  {
    {
      SMutex::Lock lock (objectsM, true, true);
      if (objects->size () < nObjects)
        break;
    }
    ::Sleep (1000);
  }

  Object* obj = new Object (this, param);
  //FIXME check creation
  objects->insert (obj);
  return obj;
}

/*template<class Object, class Parameter>
void Repository<Object, Parameter>::register_object 
  (Object* obj)
{
  SMutex::Lock lock (objectsM, true, true);

  if (objects->size () == nObjects)
    throw NoMoreObjectsPossible ();

  objects->insert (obj);
}*/

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

  if (freeMemory)
    delete obj;
}




