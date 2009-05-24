
#ifndef BOOST_MPL_INTEGRAL_C_HPP_INCLUDED
#define BOOST_MPL_INTEGRAL_C_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/integral_c.hpp,v $
// $Date: 2005/07/19 04:03:12 $
// $Revision: 1.22 $

#include <boost/mpl/integral_c_fwd.hpp>
#include <boost/mpl/aux_/config/ctps.hpp>
#include <boost/mpl/aux_/config/static_constant.hpp>
#include <boost/mpl/aux_/config/workaround.hpp>

#if BOOST_WORKAROUND(__HP_aCC, BOOST_TESTED_AT(53800))
// the type of non-type template arguments may not depend on template arguments
#   define AUX_WRAPPER_PARAMS(N) typename T, long N
#else
#   define AUX_WRAPPER_PARAMS(N) typename T, T N
#endif

#define AUX_WRAPPER_NAME integral_c
#define AUX_WRAPPER_VALUE_TYPE T
#define AUX_WRAPPER_INST(value) AUX_WRAPPER_NAME< T, value >
#include <boost/mpl/aux_/integral_wrapper.hpp>


#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
 && !BOOST_WORKAROUND(__BORLANDC__, <= 0x551)
BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN
// 'bool' constant doesn't have 'next'/'prior' members
template< bool C >
struct integral_c<bool, C>
{
    BOOST_STATIC_CONSTANT(bool, value = C);
    typedef integral_c_tag tag;
    typedef integral_c type;
    typedef bool value_type;
    operator bool() const { return this->value; }
};
BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE
#endif

#endif // BOOST_MPL_INTEGRAL_C_HPP_INCLUDED
