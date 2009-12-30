#ifndef __SNOTCOPYABLE_H
#define __SNOTCOPYABLE_H


// base for classes with disabled copy semantics

class SNotCopyable
{
public:

  SNotCopyable() {}

private:

  // no implementation - addiditional check on linkage
  SNotCopyable( const SNotCopyable & );
  SNotCopyable & operator = ( const SNotCopyable );

};


#endif  // __SNOTCOPYABLE_H
