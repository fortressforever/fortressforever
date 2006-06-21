//  (C) Copyright Gennadiy Rozental 2001-2005.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile: config.hpp,v $
//
//  Version     : $Revision: 1.2 $
//
//  Description : as a central place for global configuration switches
// ***************************************************************************

#ifndef BOOST_TEST_CONFIG_HPP_071894GER
#define BOOST_TEST_CONFIG_HPP_071894GER

// Boost
#include <boost/config.hpp> // compilers workarounds
#include <boost/detail/workaround.hpp>

#if BOOST_WORKAROUND(__GNUC__, < 3) && !defined(__SGI_STL_PORT) && !defined(_STLPORT_VERSION)
#define BOOST_CLASSIC_IOSTREAMS
#else
#define BOOST_STANDARD_IOSTREAMS
#endif

//____________________________________________________________________________//

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x570)) || \
    BOOST_WORKAROUND(__IBMCPP__, BOOST_TESTED_AT(600))     || \
    (defined __sgi && BOOST_WORKAROUND(_COMPILER_VERSION, BOOST_TESTED_AT(730)))
#define BOOST_TEST_SHIFTED_LINE
#endif

//____________________________________________________________________________//

#if defined(BOOST_MSVC) || (defined(__BORLANDC__) && !defined(BOOST_DISABLE_WIN32))
#   define BOOST_TEST_CALL_DECL __cdecl
#else
#   define BOOST_TEST_CALL_DECL /**/
#endif

//____________________________________________________________________________//

#if defined(BOOST_HAS_SIGACTION)
#define BOOST_TEST_SUPPORT_TIMEOUT
#endif

//____________________________________________________________________________//

#if BOOST_WORKAROUND(__BORLANDC__, <= 0x570)           || \
    BOOST_WORKAROUND( __COMO__, <= 0x433 )             || \
    BOOST_WORKAROUND( __INTEL_COMPILER, <= 800 )       || \
    BOOST_WORKAROUND(__GNUC__, < 3)                    || \
    defined(__sgi) && _COMPILER_VERSION <= 730         || \
    BOOST_WORKAROUND(__IBMCPP__, BOOST_TESTED_AT(600)) || \
    defined(__DECCXX) || \
    defined(__DMC__)
#define BOOST_TEST_NO_PROTECTED_USING
#endif

//____________________________________________________________________________//

#define BOOST_TEST_PROTECTED_VIRTUAL virtual

//____________________________________________________________________________//

// ***************************************************************************
//  Revision History :
//  
//  $Log: config.hpp,v $
//  Revision 1.2  2005/07/13 21:49:46  danieljames
//  Boost.Test workarounds for Digital Mars bugs.
//
//  Revision 1.1  2005/02/20 08:27:06  rogeeff
//  This a major update for Boost.Test framework. See release docs for complete list of fixes/updates
//
//  Revision 1.28  2005/02/01 06:40:07  rogeeff
//  copyright update
//  old log entries removed
//  minor stilistic changes
//  depricated tools removed
//
//  Revision 1.27  2005/01/31 07:50:06  rogeeff
//  cdecl portability fix
//
//  Revision 1.26  2005/01/30 01:48:24  rogeeff
//  BOOST_TEST_STRINGIZE introduced
//  counter type renamed
//
//  Revision 1.25  2005/01/22 19:22:12  rogeeff
//  implementation moved into headers section to eliminate dependency of included/minimal component on src directory
//
//  Revision 1.24  2005/01/21 07:33:20  rogeeff
//  BOOST_TEST_SUPPORT_TIMEOUT flag introduced to be used by used to switch code by timeout support
//
// ***************************************************************************

#endif // BOOST_TEST_CONFIG_HPP_071894GER
