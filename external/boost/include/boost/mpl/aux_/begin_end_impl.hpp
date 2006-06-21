
#ifndef BOOST_MPL_AUX_BEGIN_END_IMPL_HPP_INCLUDED
#define BOOST_MPL_AUX_BEGIN_END_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/aux_/begin_end_impl.hpp,v $
// $Date: 2004/09/02 15:40:43 $
// $Revision: 1.8 $

#include <boost/mpl/begin_end_fwd.hpp>
#include <boost/mpl/sequence_tag_fwd.hpp>
#include <boost/mpl/void.hpp>
#include <boost/mpl/aux_/na.hpp>
#include <boost/mpl/aux_/traits_lambda_spec.hpp>
#include <boost/mpl/aux_/config/eti.hpp>

namespace boost { namespace mpl {

// default implementation; conrete sequences might override it by 
// specializing either the 'begin_impl/end_impl' or the primary 
// 'begin/end' templates

template< typename Tag >
struct begin_impl
{
    template< typename Sequence > struct apply
    {
        typedef typename Sequence::begin type;
    };
};

template< typename Tag >
struct end_impl
{
    template< typename Sequence > struct apply
    {
        typedef typename Sequence::end type;
    };
};

// specialize 'begin_trait/end_trait' for two pre-defined tags

#   define AUX778076_IMPL_SPEC(name, tag, result) \
template<> \
struct name##_impl<tag> \
{ \
    template< typename Sequence > struct apply \
    { \
        typedef result type; \
    }; \
}; \
/**/

// a sequence with nested 'begin/end' typedefs; just query them
AUX778076_IMPL_SPEC(begin, nested_begin_end_tag, typename Sequence::begin)
AUX778076_IMPL_SPEC(end, nested_begin_end_tag, typename Sequence::end)

// if a type 'T' does not contain 'begin/end' or 'tag' members 
// and doesn't specialize either 'begin/end' or 'begin_impl/end_impl' 
// templates, then we end up here
AUX778076_IMPL_SPEC(begin, non_sequence_tag, void_)
AUX778076_IMPL_SPEC(end, non_sequence_tag, void_)
AUX778076_IMPL_SPEC(begin, na, void_)
AUX778076_IMPL_SPEC(end, na, void_)

#   undef AUX778076_IMPL_SPEC


BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(1,begin_impl)
BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(1,end_impl)

}}

#endif // BOOST_MPL_AUX_BEGIN_END_IMPL_HPP_INCLUDED
