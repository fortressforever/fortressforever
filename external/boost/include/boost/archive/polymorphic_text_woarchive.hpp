#ifndef BOOST_ARCHIVE_POLYMORPHIC_TEXT_WOARCHIVE_HPP
#define BOOST_ARCHIVE_POLYMORPHIC_TEXT_WOARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// polymorphic_text_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <boost/config.hpp>
#ifdef BOOST_NO_STD_WSTREAMBUF
#error "wide char i/o not supported on this platform"
#else

#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/detail/polymorphic_oarchive_impl.hpp>

namespace boost { 
namespace archive {

typedef detail::polymorphic_oarchive_impl<
        text_woarchive_impl<text_woarchive> 
> polymorphic_text_woarchive;

} // namespace archive
} // namespace boost

// required by smart_cast for compilers not implementing 
// partial template specialization
BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(
    boost::archive::polymorphic_text_woarchive
)

#endif // BOOST_NO_STD_WSTREAMBUF 
#endif // BOOST_ARCHIVE_POLYMORPHIC_TEXT_WOARCHIVE_HPP

