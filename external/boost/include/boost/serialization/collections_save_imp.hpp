#ifndef BOOST_SERIALIZATION_COLLECTIONS_SAVE_IMP_HPP
#define BOOST_SERIALIZATION_COLLECTIONS_SAVE_IMP_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// collections_save_imp.hpp: serialization for stl collections

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// helper function templates for serialization of collections

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>

namespace boost{
namespace serialization {
namespace stl {

//////////////////////////////////////////////////////////////////////
// implementation of serialization for STL containers
//

template<class Archive, class Container>
inline void save_collection(Archive & ar, const Container &s)
{
    // record number of elements
    unsigned int count = s.size();
    ar << make_nvp("count", const_cast<const unsigned int &>(count));
    BOOST_DEDUCED_TYPENAME Container::const_iterator it = s.begin();
    while(count-- > 0){
        //if(0 == (ar.get_flags() & boost::archive::no_object_creation))
                // note borland emits a no-op without the explicit namespace
                boost::serialization::save_construct_data_adl(ar, &(*it), 0U);
        ar << boost::serialization::make_nvp("item", *it++);
    }
}

} // namespace stl 
} // namespace serialization
} // namespace boost

#endif //BOOST_SERIALIZATION_COLLECTIONS_SAVE_IMP_HPP
