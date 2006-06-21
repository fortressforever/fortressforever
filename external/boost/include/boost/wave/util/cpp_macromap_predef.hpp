/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    Definition of the predefined macros
    
    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(CPP_MACROMAP_PREDEF_HPP_HK041119)
#define CPP_MACROMAP_PREDEF_HPP_HK041119

#include <cstdio>
#include <boost/assert.hpp>

#include <boost/wave/wave_config.hpp>
#include <boost/wave/token_ids.hpp>

///////////////////////////////////////////////////////////////////////////////
//
// This file contains the definition of functions needed for the management
// of static and dynamic predefined macros, such as __DATE__, __TIME__ etc.
//
// Note: __FILE__, __LINE__ and __INCLUDE_LEVEL__ are handled in the file
//       cpp_macromap.hpp.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace util {

namespace predefined_macros {

// list of static predefined macros
    struct static_macros {
        char const *name;
        boost::wave::token_id token_id;
        char const *value;
    };

// C++ mode 
    inline static_macros const & 
    static_data_cpp(std::size_t i)
    {
    static static_macros data[] = {
            { "__STDC__", T_INTLIT, "1" },
            { "__cplusplus", T_INTLIT, "199711L" },
            { 0, T_EOF, 0 }
        }; 
        BOOST_ASSERT(i < sizeof(data)/sizeof(data[0]));
        return data[i];
    }

#if BOOST_WAVE_SUPPORT_VARIADICS_PLACEMARKERS != 0
// C99 mode
    inline static_macros const &
    static_data_c99(std::size_t i)
    {
    static static_macros data[] = {
            { "__STDC__", T_INTLIT, "1" },
            { "__STDC_VERSION__", T_INTLIT, "199901L" },
            { "__STDC_HOSTED__", T_INTLIT, "0" },
            { "__WAVE_HAS_VARIADICS__", T_INTLIT, "1" },
            { 0, T_EOF, 0 }
        }; 
        BOOST_ASSERT(i < sizeof(data)/sizeof(data[0]));
        return data[i];
    }
#endif 
    
// list of dynamic predefined macros
    typedef char const * (* get_dynamic_value)(bool);

// __DATE__ 
    inline 
    char const *get_date(bool reset)
    {
    static std::string datestr;
    static const char *const monthnames[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
    
        if (reset) {
            datestr.erase();
        }
        else {
        // for some systems sprintf, time_t etc. is in namespace std
            using namespace std;    

        time_t tt = time(0);
        struct tm *tb = 0;

            if (tt != (time_t)-1) {
            char buffer[sizeof("\"Oct 11 1347\"")+1];

                tb = localtime (&tt);
                sprintf (buffer, "\"%s %2d %4d\"",
                    monthnames[tb->tm_mon], tb->tm_mday, tb->tm_year + 1900);
                datestr = buffer;
            }
            else {
                datestr = "\"??? ?? ????\"";
            }
        }
        return datestr.c_str();
    }

// __TIME__
    inline 
    char const *get_time(bool reset)
    {
    static std::string timestr;
    
        if (reset) {
            timestr.erase();
        }
        else {
        // for some systems sprintf, time_t etc. is in namespace std
            using namespace std;    

        time_t tt = time(0);
        struct tm *tb = 0;

            if (tt != (time_t)-1) {
            char buffer[sizeof("\"12:34:56\"")+1];

                tb = localtime (&tt);
                sprintf (buffer, "\"%02d:%02d:%02d\"", tb->tm_hour, 
                    tb->tm_min, tb->tm_sec);
                timestr = buffer;
            }
            else {
                timestr = "\"??:??:??\"";
            }
        }
        return timestr.c_str();
    }

// __SPIRIT_PP__/__WAVE__
    inline 
    char const *get_version(bool /*reset*/)
    {
    static std::string versionstr;
    char buffer[sizeof("0x0000")+1];

        using namespace std;    // for some systems sprintf is in namespace std
        sprintf(buffer, "0x%02d%1d%1d", BOOST_WAVE_VERSION_MAJOR, 
            BOOST_WAVE_VERSION_MINOR, BOOST_WAVE_VERSION_SUBMINOR);
        versionstr = buffer;
        return versionstr.c_str();
    }
    
// __SPIRIT_PP_VERSION__/__WAVE_VERSION__
    boost::wave::util::time_conversion_helper const 
        compilation_time(__DATE__ " " __TIME__);
        
    inline 
    char const *get_fullversion(bool /*reset*/)
    {
    static std::string versionstr;
    char buffer[sizeof("0x00000000")+1];

    // for some systems sprintf, time_t etc. is in namespace std
        using namespace std;    

    // calculate the number of days since Dec 13 2001 
    // (the day the Wave project was started)
    tm first_day;

        using namespace std;    // for some systems memset is in namespace std
        memset (&first_day, 0, sizeof(tm));
        first_day.tm_mon = 11;           // Dec
        first_day.tm_mday = 13;          // 13
        first_day.tm_year = 101;         // 2001

    long seconds = long(difftime(compilation_time.get_time(), 
        mktime(&first_day)));

        sprintf(buffer, "0x%02d%1d%1d%04ld", BOOST_WAVE_VERSION_MAJOR,
             BOOST_WAVE_VERSION_MINOR, BOOST_WAVE_VERSION_SUBMINOR, 
             seconds/(3600*24));
        versionstr = buffer;
        return versionstr.c_str();
    }
    
// __SPIRIT_PP_VERSION_STR__/__WAVE_VERSION_STR__
    inline 
    char const *get_versionstr(bool /*reset*/)
    {
    static std::string versionstr;
    char buffer[sizeof("\"00.00.00.0000\"")+1];

    // for some systems sprintf, time_t etc. is in namespace std
        using namespace std;    

    // calculate the number of days since Dec 13 2001 
    // (the day the Wave project was started)
    tm first_day;

        using namespace std;    // for some systems memset is in namespace std
        memset (&first_day, 0, sizeof(tm));
        first_day.tm_mon = 11;           // Dec
        first_day.tm_mday = 13;          // 13
        first_day.tm_year = 101;         // 2001

    long seconds = long(difftime(compilation_time.get_time(), 
        mktime(&first_day)));

        sprintf(buffer, "\"%d.%d.%d.%ld\"", BOOST_WAVE_VERSION_MAJOR,
             BOOST_WAVE_VERSION_MINOR, BOOST_WAVE_VERSION_SUBMINOR, 
             seconds/(3600*24));
        versionstr = buffer;
        return versionstr.c_str();
    }
    
    struct dynamic_macros {
        char const *name;
        boost::wave::token_id token_id;
        get_dynamic_value generator;
    };
    
    inline dynamic_macros const &
    dynamic_data(std::size_t i)
    {
    static dynamic_macros data[] = {
            { "__DATE__", T_STRINGLIT, get_date },
            { "__TIME__", T_STRINGLIT, get_time },
            { "__SPIRIT_PP__", T_INTLIT, get_version },
            { "__SPIRIT_PP_VERSION__", T_INTLIT, get_fullversion },
            { "__SPIRIT_PP_VERSION_STR__", T_STRINGLIT, get_versionstr },
            { "__WAVE__", T_INTLIT, get_version },
            { "__WAVE_VERSION__", T_INTLIT, get_fullversion },
            { "__WAVE_VERSION_STR__", T_STRINGLIT, get_versionstr },
            { 0, T_EOF, 0 }
        };
        BOOST_ASSERT(i < sizeof(data)/sizeof(data[0]));
        return data[i];
    }
    
}   // namespace predefined_macros

///////////////////////////////////////////////////////////////////////////////
}   // namespace util
}   // namespace wave
}   // namespace boost

#endif // !defined(CPP_MACROMAP_PREDEF_HPP_HK041119)
