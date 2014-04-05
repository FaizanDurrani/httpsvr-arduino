///////////////////////////////////////////////////////////////////////////////
//
//  vdata.h - Definition of class vdata
//
//  The vdata class template is a type decorator which provides valid/invalid
//  semantics to a given object.
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTVDATA_H_
#define _EXTVDATA_H_

#include <stdexcept>

namespace ext {
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class vdata
{
public:

  typedef T                   data_type;

  ////////////////////////////////////////////////
  
  vdata() : my_data(), my_vflag(false) {}

  // vdata(const vdata<data_type>& the_other) --> implicit
  // vdata<data_type>& operator=(const vdata<data_type>& the_other) --> implicit

  explicit vdata              (const data_type& the_data) : my_data(the_data), my_vflag(true) {}   // vdata<data_type> aProp(aData);
  vdata<data_type>& operator= (const data_type& the_data) { my_data = the_data; my_vflag = true; return *this; } // aVData = aData;

  virtual ~vdata() {}

public:
  ////////////////////////////////////////////////
  // Non-mutating access functions
  operator const data_type&   () const                    { return private_data(); } // if (aData == aVData)...; aData = aVData; aData += aVData; etc.
  const data_type&       get  () const                    { return private_data(); } // aVData.get().constMemberFn(); aTemp = aVData.get().myMember;
  const data_type&       data () const                    { return get(); }

  ////////////////////////////////////////////////
  // Mutating and non-const access functions
  operator data_type&         ()                          { return private_nc_data(); } // aVData += aData
  data_type&          nc_get  ()                          { return private_nc_data(); } // aVData.vref_t().mutatingMemberFn(); aVData.nc_get().myMember = aTemp;
  data_type&          nc_data ()                          { return nc_get(); }
  void                set     (const data_type& the_data) { *this = the_data; }         // aVData.set(aData);
  void                reset   ()                          { *this = vdata<T>(); }

  ////////////////////////////////////////////////
  // Status test/set
  bool                isValid () const                    { return my_vflag; }

private:
  const data_type& private_data() const
  { if (isValid()) return my_data; throw std::runtime_error(__FUNCTION__) << " - Invalid data"; }

  data_type& private_nc_data()                    
  { return const_cast<data_type&>(static_cast<const vdata<T>&>(*this).private_data()); }

private:
  data_type my_data;
  bool      my_vflag;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace ext

#endif // #ifndef _EXTVDATA_H_

