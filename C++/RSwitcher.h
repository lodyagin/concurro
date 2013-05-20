#ifndef RSWITCHER_H_
#define RSWITCHER_H_

#include <cstdatomic>

/**
 * A value with wait-free update implementation.
 */
template<class T, class TUpdate>
class RSwitcher
{
public:
  /// Update the element.
  void update(const TUpdate&);
  /// Get the last updated state.
  T get() const;
};

#endif
