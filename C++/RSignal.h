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

#include "Repository.h"
#include "SSingleton.h"
#include <unordered_map>

namespace curr {

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

}
