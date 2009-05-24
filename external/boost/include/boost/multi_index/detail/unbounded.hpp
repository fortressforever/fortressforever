/* Copyright 2003-2005 Joaqu�n M L�pez Mu�oz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_UNBOUNDED_HPP
#define BOOST_MULTI_INDEX_DETAIL_UNBOUNDED_HPP

#if defined(_MSC_VER)&&(_MSC_VER>=1200)
#pragma once
#endif

namespace boost{

namespace multi_index{

/* dummy type and variable for use in ordered_index::range() */

namespace detail{

struct unbounded_type{};

} /* namespace multi_index::detail */

namespace{

detail::unbounded_type  unbounded_obj=detail::unbounded_type();
detail::unbounded_type& unbounded=unbounded_obj;

} /* unnamed */

} /* namespace multi_index */

} /* namespace boost */

#endif
