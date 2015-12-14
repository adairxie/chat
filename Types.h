#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <string>
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>

#include <assert.h>
using namespace std;

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
   return f;
}

template<typename To,typename From>
inline To down_cast(From* f)
{
  if(false)
  {
     implicit_cast<From*,To>(0);
  }

  return static_cast<To>(f);
}


#endif
