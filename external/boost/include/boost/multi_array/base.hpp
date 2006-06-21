// Copyright 2002 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

#ifndef BASE_RG071801_HPP
#define BASE_RG071801_HPP

//
// base.hpp - some implementation base classes for from which
// functionality is acquired
//

#include "boost/multi_array/extent_range.hpp"
#include "boost/multi_array/extent_gen.hpp"
#include "boost/multi_array/index_range.hpp"
#include "boost/multi_array/index_gen.hpp"
#include "boost/multi_array/storage_order.hpp"
#include "boost/multi_array/types.hpp"
#include "boost/config.hpp"
#include "boost/mpl/eval_if.hpp"
#include "boost/mpl/if.hpp"
#include "boost/mpl/size_t.hpp"
#include "boost/mpl/aux_/msvc_eti_base.hpp"
#include "boost/iterator/reverse_iterator.hpp"
#include "boost/static_assert.hpp"
#include "boost/type.hpp"
#include <cassert>
#include <cstddef>
#include <memory>

namespace boost {

/////////////////////////////////////////////////////////////////////////
// class declarations
/////////////////////////////////////////////////////////////////////////

template<typename T, std::size_t NumDims,
  typename Allocator = std::allocator<T> >
class multi_array;

// This is a public interface for use by end users!
namespace multi_array_types {
  typedef boost::detail::multi_array::size_type size_type;
  typedef std::ptrdiff_t difference_type;
  typedef boost::detail::multi_array::index index;
  typedef detail::multi_array::index_range<index,size_type> index_range;
  typedef detail::multi_array::extent_range<index,size_type> extent_range;
  typedef detail::multi_array::index_gen<0,0> index_gen;
  typedef detail::multi_array::extent_gen<0> extent_gen;
}


// boost::extents and boost::indices are now a part of the public
// interface.  That way users don't necessarily have to create their 
// own objects.  On the other hand, one may not want the overhead of 
// object creation in small-memory environments.  Thus, the objects
// can be left undefined by defining BOOST_MULTI_ARRAY_NO_GENERATORS 
// before loading multi_array.hpp.
#if !BOOST_MULTI_ARRAY_NO_GENERATORS
namespace {
  multi_array_types::extent_gen extents;
  multi_array_types::index_gen indices;
}
#endif // BOOST_MULTI_ARRAY_NO_GENERATORS

namespace detail {
namespace multi_array {

template <typename T, std::size_t NumDims>
class sub_array;

template <typename T, std::size_t NumDims, typename TPtr = const T*>
class const_sub_array;

template <typename T, typename TPtr, typename NumDims, typename Reference>
class array_iterator;

template <typename T, std::size_t NumDims, typename TPtr = const T*>
class const_multi_array_view;

template <typename T, std::size_t NumDims>
class multi_array_view;

/////////////////////////////////////////////////////////////////////////
// class interfaces
/////////////////////////////////////////////////////////////////////////

class multi_array_base {
public:
  typedef multi_array_types::size_type size_type;
  typedef multi_array_types::difference_type difference_type;
  typedef multi_array_types::index index;
  typedef multi_array_types::index_range index_range;
  typedef multi_array_types::extent_range extent_range;
  typedef multi_array_types::index_gen index_gen;
  typedef multi_array_types::extent_gen extent_gen;
};

//
// value_accessor_n
//  contains the routines for accessing elements from
//  N-dimensional views.
//
template<typename T, std::size_t NumDims>
class value_accessor_n : public multi_array_base {
  typedef multi_array_base super_type;
public:
  typedef typename super_type::index index;

  // 
  // public typedefs used by classes that inherit from this base
  //
  typedef T element;
  typedef boost::multi_array<T,NumDims-1> value_type;
  typedef sub_array<T,NumDims-1> reference;
  typedef const_sub_array<T,NumDims-1> const_reference;

protected:
  // used by array operator[] and iterators to get reference types.
  template <typename Reference, typename TPtr>
  Reference access(boost::type<Reference>,index idx,TPtr base,
                   const size_type* extents,
                   const index* strides,
                   const index* index_base) const {

    // return a sub_array<T,NDims-1> proxy object
    TPtr newbase = base + idx * strides[0];
    return Reference(newbase,extents+1,strides+1,index_base+1);

  }

  value_accessor_n() { }
  ~value_accessor_n() { }
};



//
// value_accessor_one
//  contains the routines for accessing reference elements from
//  1-dimensional views.
//
template<typename T>
class value_accessor_one : public multi_array_base {
  typedef multi_array_base super_type;
public:
  typedef typename super_type::index index;
  //
  // public typedefs for use by classes that inherit it.
  //
  typedef T element;
  typedef T value_type;
  typedef T& reference;
  typedef T const& const_reference;

protected:
  // used by array operator[] and iterators to get reference types.
  template <typename Reference, typename TPtr>
  Reference access(boost::type<Reference>,index idx,TPtr base,
                   const size_type*,
                   const index* strides,
                   const index*) const {
    return *(base + idx * strides[0]);
  }

  value_accessor_one() { }
  ~value_accessor_one() { }
};


/////////////////////////////////////////////////////////////////////////
// choose value accessor begins
//

template <typename T, std::size_t NumDims>
struct choose_value_accessor_n {
  typedef value_accessor_n<T,NumDims> type;
};

template <typename T>
struct choose_value_accessor_one {
  typedef value_accessor_one<T> type;
};

template <typename T, typename NumDims>
struct value_accessor_generator {
    BOOST_STATIC_CONSTANT(std::size_t, dimensionality = NumDims::value);
    
  typedef typename
  mpl::eval_if_c<(dimensionality == 1),
                  choose_value_accessor_one<T>,
                  choose_value_accessor_n<T,dimensionality>
  >::type type;
};

#if BOOST_WORKAROUND(BOOST_MSVC, == 1200)

struct eti_value_accessor
{
  typedef int index;
  typedef int size_type;
  typedef int element;
  typedef int index_range;
  typedef int value_type;
  typedef int reference;
  typedef int const_reference;
};
    
template <>
struct value_accessor_generator<int,int>
{
  typedef eti_value_accessor type;
};

template <class T, class NumDims>
struct associated_types
  : mpl::aux::msvc_eti_base<
        typename value_accessor_generator<T,NumDims>::type
    >::type
{};

template <>
struct associated_types<int,int> : eti_value_accessor {};

#else

template <class T, class NumDims>
struct associated_types
  : value_accessor_generator<T,NumDims>::type
{};

#endif

//
// choose value accessor ends
/////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// multi_array_base
////////////////////////////////////////////////////////////////////////
template <typename T, std::size_t NumDims>
class multi_array_impl_base
  :
#if BOOST_WORKAROUND(BOOST_MSVC, == 1200)
      public mpl::aux::msvc_eti_base<
          typename value_accessor_generator<T,mpl::size_t<NumDims> >::type
       >::type
#else
      public value_accessor_generator<T,mpl::size_t<NumDims> >::type
#endif 
{
  typedef associated_types<T,mpl::size_t<NumDims> > types;
public:
  typedef typename types::index index;
  typedef typename types::size_type size_type;
  typedef typename types::element element;
  typedef typename types::index_range index_range;
  typedef typename types::value_type value_type;
  typedef typename types::reference reference;
  typedef typename types::const_reference const_reference;

  template <std::size_t NDims>
  struct subarray {
    typedef boost::detail::multi_array::sub_array<T,NDims> type;
  };

  template <std::size_t NDims>
  struct const_subarray {
    typedef boost::detail::multi_array::const_sub_array<T,NDims> type;
  };

  template <std::size_t NDims>
  struct array_view {
    typedef boost::detail::multi_array::multi_array_view<T,NDims> type;
  };

  template <std::size_t NDims>
  struct const_array_view {
  public:
    typedef boost::detail::multi_array::const_multi_array_view<T,NDims> type;
  };

  //
  // iterator support
  //
  typedef array_iterator<T,T*,mpl::size_t<NumDims>,reference> iterator;
  typedef array_iterator<T,T const*,mpl::size_t<NumDims>,const_reference> const_iterator;

  typedef ::boost::reverse_iterator<iterator> reverse_iterator;
  typedef ::boost::reverse_iterator<const_iterator> const_reverse_iterator;

  BOOST_STATIC_CONSTANT(std::size_t, dimensionality = NumDims);
protected:

  multi_array_impl_base() { }
  ~multi_array_impl_base() { }

  // Used by operator() in our array classes
  template <typename Reference, typename IndexList, typename TPtr>
  Reference access_element(boost::type<Reference>, TPtr base,
                           const IndexList& indices,
                           const index* strides) const {
    index offset = 0;
    for (size_type n = 0; n != NumDims; ++n) 
      offset += indices[n] * strides[n];
    
    return base[offset];
  }

  template <typename StrideList, typename ExtentList>
  void compute_strides(StrideList& stride_list, ExtentList& extent_list,
                       const general_storage_order<NumDims>& storage)
  {
    // invariant: stride = the stride for dimension n
    index stride = 1;
    for (size_type n = 0; n != NumDims; ++n) {
      index stride_sign = +1;
      
      if (!storage.ascending(storage.ordering(n)))
        stride_sign = -1;
      
      // The stride for this dimension is the product of the
      // lengths of the ranks minor to it.
      stride_list[storage.ordering(n)] = stride * stride_sign;
      
      stride *= extent_list[storage.ordering(n)];
    } 
  }

  // This calculates the offset to the array base pointer due to:
  // 1. dimensions stored in descending order
  // 2. non-zero dimension index bases
  template <typename StrideList, typename ExtentList, typename BaseList>
  index
  calculate_origin_offset(const StrideList& stride_list,
                          const ExtentList& extent_list,
                          const general_storage_order<NumDims>& storage,
                          const BaseList& index_base_list)
  {
    return
      calculate_descending_dimension_offset(stride_list,extent_list,
                                            storage) +
      calculate_indexing_offset(stride_list,index_base_list);
  }

  // This calculates the offset added to the base pointer that are
  // caused by descending dimensions
  template <typename StrideList, typename ExtentList>
  index
  calculate_descending_dimension_offset(const StrideList& stride_list,
                                const ExtentList& extent_list,
                                const general_storage_order<NumDims>& storage)
  {
    index offset = 0;
    if (!storage.all_dims_ascending()) 
      for (size_type n = 0; n != NumDims; ++n)
        if (!storage.ascending(n))
          offset -= (extent_list[n] - 1) * stride_list[n];

    return offset;
  }

  // This is used to reindex array_views, which are no longer
  // concerned about storage order (specifically, whether dimensions
  // are ascending or descending) since the viewed array handled it.

  template <typename StrideList, typename BaseList>
  index
  calculate_indexing_offset(const StrideList& stride_list,
                          const BaseList& index_base_list)
  {
    index offset = 0;
    for (size_type n = 0; n != NumDims; ++n)
        offset -= stride_list[n] * index_base_list[n];
    return offset;
  }

  // Slicing using an index_gen.
  // Note that populating an index_gen creates a type that encodes
  // both the number of dimensions in the current Array (NumDims), and 
  // the Number of dimensions for the resulting view.  This allows the 
  // compiler to fail if the dimensions aren't completely accounted
  // for.  For reasons unbeknownst to me, a BOOST_STATIC_ASSERT
  // within the member function template does not work. I should add a 
  // note to the documentation specifying that you get a damn ugly
  // error message if you screw up in your slicing code.
  template <typename ArrayRef, int NDims, typename TPtr>
  ArrayRef
  generate_array_view(boost::type<ArrayRef>,
                      const boost::detail::multi_array::
                      index_gen<NumDims,NDims>& indices,
                      const size_type* extents,
                      const index* strides,
                      const index* index_bases,
                      TPtr base) const {

    boost::array<index,NDims> new_strides;
    boost::array<index,NDims> new_extents;

    index offset = 0;
    size_type dim = 0;
    for (size_type n = 0; n != NumDims; ++n) {
      const index default_start = index_bases[n];
      const index default_finish = default_start+extents[n];
      const index_range& current_range = indices.ranges_[n];
      index start = current_range.get_start(default_start);
      index finish = current_range.get_finish(default_finish);
      index index_factor = current_range.stride();
      index len = (finish - start + (index_factor - 1)) / index_factor;

      // the array data pointer is modified to account for non-zero
      // bases during slicing (see [Garcia] for the math involved)
      offset += start * strides[n];

      if (!current_range.is_degenerate()) {

        // The index_factor for each dimension is included into the
        // strides for the array_view (see [Garcia] for the math involved).
        new_strides[dim] = index_factor * strides[n];
        
        // calculate new extents
        new_extents[dim] = len;
        ++dim;
      }
    }
    assert (dim == NDims);

    return
      ArrayRef(base+offset,
               new_extents,
               new_strides);
  }
                     

};

} // namespace multi_array
} // namespace detail

} // namespace boost

#endif // BASE_RG071801_HPP
