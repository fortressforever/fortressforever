#ifndef BOOST_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP
#define BOOST_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// type_info_implementation.hpp: interface for portable version of type_info

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.


#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

namespace boost {
namespace serialization {
template<class T>
class extended_type_info_null;
struct basic_traits;
} // namespace serialization
} // namespace boost


#ifdef BOOST_SERIALIZATION_DEFAULT_TYPE_INFO
    #define BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_STUB(T)        \
        BOOST_SERIALIZATION_DEFAULT_TYPE_INFO(T)
#else
    #define BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_STUB(T)        \
        extended_type_info_null< T >
#endif

#include <boost/static_assert.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/serialization/traits.hpp>

namespace boost {
namespace serialization {

// note that T and const T are folded into const T so that
// there is only one table entry per type
template<class T>
struct type_info_implementation {
    template<class U>
    struct traits_class_typeinfo_implementation {
        typedef BOOST_DEDUCED_TYPENAME U::type_info_implementation type;
    };
    typedef 
        BOOST_DEDUCED_TYPENAME mpl::eval_if<
            boost::is_base_and_derived<basic_traits, T>,
            traits_class_typeinfo_implementation<T>,
        //else
            mpl::identity<
                BOOST_SERIALIZATION_EXTENDED_TYPE_INFO_STUB(T)
            >
        >::type type;
};

} // namespace serialization
} // namespace boost

// define a macro to assign a particular derivation of extended_type_info
// to a specified a class. 
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x560))
#define BOOST_CLASS_TYPE_INFO(T, ETI)              \
namespace boost {                                  \
namespace serialization {                          \
template<>                                         \
struct type_info_implementation< T > {             \
    typedef ETI type;                              \
};                                                 \
}                                                  \
}                                                  \
/**/
#else
#define BOOST_CLASS_TYPE_INFO(T, ETI)              \
namespace boost {                                  \
namespace serialization {                          \
template<>                                         \
struct type_info_implementation< T > {             \
    typedef ETI type;                              \
};                                                 \
template<>                                         \
struct type_info_implementation< const T > {       \
    typedef ETI type;                              \
};                                                 \
}                                                  \
}                                                  \
/**/
#endif

#endif /// BOOST_SERIALIZATION_TYPE_INFO_IMPLEMENTATION_HPP
