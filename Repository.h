#pragma once
#include "smutex.h"
#include "snotcopyable.h"
#include "sexception.h"
#include "SShutdown.h"
#include "SEvent.h"
#include "SThread.h"
#include <vector>

template<class Object, class Parameter>
class Repository : public SNotCopyable
{
protected:
  typedef std::vector<Object*> ObjectMap;

public:
  typedef typename ObjectMap::size_type ObjectId;

  Repository (int maxNumberOfObjects);
  ~Repository ();

  virtual Object* create_object (const Parameter& param);
  virtual void delete_object (Object* obj, bool freeMemory);
  virtual Object* get_object_by_id (ObjectId id);

  // It is used for object creation
  struct ObjectCreationInfo
  {
    //const Parameter* info;
    Repository* repository;
    std::string objectId;
  };

protected:
  static ObjectId get_first_unused_object_id (ObjectMap&);

  enum {startMapSize = 10, mapSizeStep = 10};

  ObjectMap* objects;
  SMutex objectsM;
  SSemaphore semaphore;
};

template<class Object, class Parameter>
Repository<Object, Parameter>::Repository 
  (int maxNumberOfObjects)
: objects (NULL), 
  semaphore (maxNumberOfObjects, maxNumberOfObjects)
{
  objects = new ObjectMap ();
  objects->reserve (startMapSize);
  //FIXME check obj creation
  objects->push_back (0); // obj id 0 is not used
}

template<class Object, class Parameter>
Repository<Object, Parameter>::~Repository ()
{
  //FIXME add some code...
  delete objects;
}

template<class Object, class Parameter>
Object* Repository<Object, Parameter>::create_object 
  (const Parameter& param)
{
  semaphore.wait ();

  if (SThread::current ().is_stop_requested ())
       ::xShuttingDown 
        (L"Stop request from the owner thread.");

  { 
    SMutex::Lock lock (objectsM, true, true);

    const ObjectId objId = get_first_unused_object_id 
      (*objects);

    std::string uniId;
    toString (objId, uniId);
    const ObjectCreationInfo cinfo =
      { this, uniId };

    Object* obj = param.create_derivation (cinfo);
    //FIXME check creation

    objects->at (objId) = obj;
    return obj;
  }
}

template<class Object, class Parameter>
void Repository<Object, Parameter>::delete_object 
  (Object* obj, 
   bool freeMemory
   )
{
  const ObjectId objId = fromString<ObjectId> (obj->universal_object_id);

  {
    SMutex::Lock lock (objectsM, true, true);

    ObjectMap::reference r = objects->at (objId);
    if (r == 0)
    {
      THROW_EXCEPTION
      (SException, oss_ << L"Program error");
    }

    r = 0;
  }

  semaphore.release ();

  if (freeMemory)
    THROW_EXCEPTION
      (SException, oss_ << "Not implemented");
}


template<class Object, class Parameter>
Object* Repository<Object, Parameter>::get_object_by_id 
  (typename Repository<Object, Parameter>::ObjectId id)
{
  { 
    SMutex::Lock lock (objectsM, true, true);

    if (id < 1 || id >= objects->size ())
      return 0;
    else
      return objects->at (id);
  }
}


template<class Object, class Parameter>
typename Repository<Object, Parameter>::ObjectId 
Repository<Object, Parameter>::get_first_unused_object_id
  (ObjectMap& m)
{ //TODO UT

  for (ObjectId id = 1; id < m.size (); id++)
  {
    if (!m[id]) return id;
  }

  if (m.size () == m.capacity ())
    m.reserve (m.size () + mapSizeStep);
  // TODO check the stepping

  m.push_back (0);
  return m.size () - 1;
}





