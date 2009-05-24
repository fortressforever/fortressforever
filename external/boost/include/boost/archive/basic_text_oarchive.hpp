#ifndef BOOST_ARCHIVE_BASIC_TEXT_OARCHIVE_HPP
#define BOOST_ARCHIVE_BASIC_TEXT_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_text_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// archives stored as text - note these ar templated on the basic
// stream templates to accommodate wide (and other?) kind of characters
//
// note the fact that on libraries without wide characters, ostream is
// is not a specialization of basic_ostream which in fact is not defined
// in such cases.   So we can't use basic_ostream<OStream::char_type> but rather
// use two template parameters

#include <cassert>
#include <boost/config.hpp>
#include <boost/pfto.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/archive/detail/oserializer.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>

#include <boost/serialization/string.hpp>

#include <boost/archive/detail/abi_prefix.hpp> // must be the last header

namespace boost {
namespace archive {

/////////////////////////////////////////////////////////////////////////
// class basic_text_iarchive - read serialized objects from a input text stream
template<class Archive>
class basic_text_oarchive : 
    public detail::common_oarchive<Archive>
{
#if BOOST_WORKAROUND(BOOST_MSVC, <= 1300) \
|| BOOST_WORKAROUND(__BORLANDC__,BOOST_TESTED_AT(0x560))
public:
#elif defined(BOOST_MSVC)
    // for some inexplicable reason insertion of "class" generates compile erro
    // on msvc 7.1
    friend detail::interface_oarchive<Archive>;
protected:
#else
    friend class detail::interface_oarchive<Archive>;
protected:
#endif
    enum {
        none,
        eol,
        space
    } delimiter;

    void newline(){
        delimiter = eol;
    }

    BOOST_ARCHIVE_OR_WARCHIVE_DECL(void)
    newtoken();

    // default processing - invoke serialization library
    template<class T>
    void save_override(T & t, BOOST_PFTO int)
    {
        archive::save(* this->This(), t);
    }

    // start new objects on a new line
    void save_override(const object_id_type & t, int){
        this->This()->newline();
        // and and invoke prmitive to underlying value
        this->This()->save(t.t);
    }

    void save_override(const object_reference_type & t, int){
        this->This()->newline();
        // and and invoke prmitive to underlying value
        this->This()->save(t.t);
    }

    // text file don't include the optional information 
    void save_override(const class_id_optional_type & /* t */, int){}

    void save_override(const class_name_type & t, int){
                const std::string s(t);
                * this->This() << s;
    }

    BOOST_ARCHIVE_OR_WARCHIVE_DECL(void)
    init();

    basic_text_oarchive(unsigned int flags) :
        detail::common_oarchive<Archive>(flags),
        delimiter(none)
    {}

    ~basic_text_oarchive(){}
};

} // namespace archive
} // namespace boost

#include <boost/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_ARCHIVE_BASIC_TEXT_OARCHIVE_HPP
