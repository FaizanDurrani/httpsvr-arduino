///////////////////////////////////////////////////////////////////////////////
//
//  vinit.h - Definition of class vinit
//
//  The vinit class template provides value initialization semantics
//  to the object it is applied to. It is particularly useful when applied
//  to POD types
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTVINIT_H_
#define _EXTVINIT_H_

namespace ext {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct vinit
{
  typedef T value_type;
  static T zero() { return T(); }

  vinit() : my_value() {}
  vinit(const T& v) : my_value(v) {}
  vinit(const vinit<T>& vi) : my_value(vi.my_value) {}
  vinit<T>& operator=(const vinit<T>& vi) { my_value = vi.my_value; return *this; }
  vinit<T>& operator=(const T& v) { my_value = v; return *this; }
  operator T&() { return my_value; }
  operator const T&() const { return my_value; }

private:
  T my_value;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace ext

#endif // #ifndef _EXTVINIT_H_

