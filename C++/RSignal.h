// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "Repository.h"
#include "SSingleton.h"
#include <unordered_map>

enum class RSignalAction { Ignore, Process };

class RSignalBase
{
public:
  struct Par
  {
	 int signum;
	 RSignalAction action;

    Par(int signum_, RSignalAction action_) 
	 : signum(signum_, action_) {}

	 virtual RSignalBase* create_derivation
	 (const ObjectCreationInfo& oi) const;

	 virtual RSignalBase* transform_object 
	 (const RSignalBase*) const;

	 int get_id() const { return signum; }
  };

  //! What to do whith this signal
  const int signum;
  const RSignalAction action;

protected:
  RSignalBase(const ObjectCreationInfo& oi,
				  const Par& par);
  virtual ~RSignalBase() {}
};

class RIgnoredSignal : public RSignalBase {};

class RSignalHandler : public RSignalBase 
{
public:
  
};

class RSignalRepository 
  : public Repository<
      RSignalBase, RSignalBase::Par, 
      std::unordered_map, int
  >,
  public SAutoSingleton<RSignalRepository>
{
public:
  void tune_signal(int signum, RSignalAction action);
};

template<int signum>
class RSignal //: public RSignalBase
{
public:
  void tune_signal(RSignalAction action)
  {
	 RSignalRepository::instance()
		. tune_signal(signum, action);
  }
};

