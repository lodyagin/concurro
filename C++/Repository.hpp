// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId 
>
RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::~RepositoryBase ()
{
  std::for_each
    (objects->begin (), 
     objects->end (),
     Destructor<Obj, Par, ObjMap, ObjId, typename ObjMap::value_type> (this)
     );
  delete objects;
}

template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId
>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::create_object (const Par& param)
{
  /*if (SThread::current ().is_stop_requested ())
       ::xShuttingDown 
        (L"Stop request from the owner thread.");*/
  Obj* obj = 0;
  { 
    RLOCK (objectsM);

    const ObjId objId = get_object_id(param);

    std::string uniId;
    toString (objId, uniId);
    const ObjectCreationInfo cinfo = { this, uniId };

	 // dynamic cast for use with inherited parameters
    obj = dynamic_cast<Obj*>
		(param.create_derivation (cinfo));

    SCHECK (obj);
    insert_object (objId, obj);
  }
  LOG_DEBUG(log, "Object " << *obj << " is created.");
  return obj;
}

template<class Obj, class Par, class ObjMap, class ObjId>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::replace_object (ObjId id, const Par& param, bool freeMemory)
{
  RLOCK(objectsM);

  Obj* obj = 0;
  try {
	 obj = objects->at (id);
  }
  catch (const std::out_of_range&) {
	 THROW_EXCEPTION(NoSuchId, id);
  }

  if (!obj) THROW_PROGRAM_ERROR;
  //obj->freeze();

  (*objects)[id] = dynamic_cast<Obj*>
	 (param.transform_object (obj));

  SCHECK ((*objects)[id]);

  if ((*objects)[id] == obj)
    return obj; // no transformation

  if (freeMemory)
    delete obj;

  return (*objects)[id];
}

template<class Obj, class Par, class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object (Obj* obj, bool freeMemory)
{
  assert (obj);
  const ObjId objId = fromString<ObjId> 
	 (obj->universal_id());

  delete_object_by_id (objId, freeMemory);
}

template<class Obj, class Par, class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object_by_id (ObjId id, bool freeMemory)
{
  Obj* ptr = 0;
  {
    RLOCK(objectsM);

	 try {
		Obj*& r = objects->at (id);
		ptr = r;
		if (r == 0) THROW_PROGRAM_ERROR;
		r = 0;
	 }
	 catch (const std::out_of_range&) {
		THROW_EXCEPTION(NoSuchId, id);
	 }
  }
  //ptr->freeze();
  if (freeMemory) delete ptr;
}

template <class Obj, class Par, class ObjMap, class ObjId>
Obj* RepositoryBase <Obj, Par, ObjMap, ObjId>
//
::get_object_by_id (ObjId id) const
{
  try { 
    RLOCK(objectsM);
	 return objects->at (id);
  }
  catch (const std::out_of_range&)
  {
	 THROW_EXCEPTION(NoSuchId, id);
  }
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Out, class Pred>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_pred (Out res, Pred p)
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i] && p (*(*objects)[i]))
      *res++ = i;
  return res;
}

template<class Obj, class State>
class StateMatch 
  : public std::unary_function<const Obj&, bool>
{
public:
  StateMatch (const State& _state) : state (_state) {}
  bool operator () (const Obj& obj) const
  { 
    return State::state_is (obj, state);
  }
protected:
  State state;
};

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Out, class State>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_state (Out res, const State& state)
{
  return get_object_ids_by_pred<Out, StateMatch<Obj,
  State>> 
	 (res, StateMatch<Obj, State> (state));
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Op>
void RepositoryBase<Obj, Par, ObjMap, ObjId>::for_each (Op& f)
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i])
      f (*(*objects)[i]);
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Op>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::for_each (Op& f) const
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i])
      f (*(*objects)[i]);
}

