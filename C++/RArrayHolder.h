/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RARRAYHOLDER_H_
#define CONCURRO_RARRAYHOLDER_H_

namespace curr {

template
<
  class T,
  class Obj = T,

  template<class, int>
  class Guard = T::template guard_templ,

  int wait_m = 1000
>
class RArrayHolder 
  : public T::states_interface,
    public RHolderBase
{
public:
  typedef typename Obj::Par Par;
  
};


} // namespace curr

#endif

