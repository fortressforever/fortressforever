
#ifndef BOOST_MPL_SET_AUX_SIZE_IMPL_HPP_INCLUDED
#define BOOST_MPL_SET_AUX_SIZE_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/boost/mpl/set/aux_/size_impl.hpp,v $
// $Date: 2004/09/02 15:41:02 $
// $Revision: 1.2 $

#include <boost/mpl/size_fwd.hpp>
#include <boost/mpl/set/aux_/tag.hpp>

namespace boost { namespace mpl {

template<>
struct size_impl< aux::set_tag >
{
    template< typename Set > struct apply
        : Set::size
    {
    };
};

}}

#endif // BOOST_MPL_SET_AUX_SIZE_IMPL_HPP_INCLUDED
