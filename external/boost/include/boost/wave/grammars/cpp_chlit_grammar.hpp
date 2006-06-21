/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(CPP_CHLIT_GRAMMAR_HPP_9527D349_6592_449A_A409_42A001E6C64C_INCLUDED)
#define CPP_CHLIT_GRAMMAR_HPP_9527D349_6592_449A_A409_42A001E6C64C_INCLUDED

#include <limits>     // sid::numeric_limits

#include <boost/static_assert.hpp>
#include <boost/cstdint.hpp>

#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/dynamic/if.hpp>

#include <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix/primitives.hpp>
#include <boost/spirit/phoenix/statements.hpp>
#include <boost/spirit/phoenix/functions.hpp>

#include <boost/wave/wave_config.hpp>   
#include <boost/wave/cpp_exceptions.hpp>   
#include <boost/wave/grammars/cpp_literal_grammar_gen.hpp>

///////////////////////////////////////////////////////////////////////////////
//
//  Reusable grammar to parse a C++ style character literal 
//
///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave { 
namespace grammars {

namespace closures {

    struct chlit_closure 
    :   boost::spirit::closure<chlit_closure, boost::uint32_t, bool> 
    {
        member1 value;
        member2 long_lit;
    };
}

namespace impl {

///////////////////////////////////////////////////////////////////////////////
//
//  compose a multibyte character literal
//
///////////////////////////////////////////////////////////////////////////////
    struct compose_character_literal {

        template <typename ResultT, typename A1, typename A2, typename A3>
        struct result { 
        
            typedef void type; 
        };

        void 
        operator()(boost::uint32_t& value, bool long_lit, bool& overflow,
            boost::uint32_t character) const
        { 
            // The following assumes that wchar_t is max. 32 Bit
            BOOST_STATIC_ASSERT(sizeof(wchar_t) <= 4);
            
            static boost::uint32_t masks[] = { 
                0x000000ff, 0x0000ffff, 0x00ffffff, 0xffffffff
            };
            static boost::uint32_t overflow_masks[] = { 
                0xff000000, 0xffff0000, 0xffffff00, 0xffffffff
            };
            
            if (long_lit) {
            // make sure no overflow will occur below
                if ((value & overflow_masks[sizeof(wchar_t)-1]) != 0) {
                    overflow |= true;
                }
                else {
                // calculate the new value (avoiding a warning regarding 
                // shifting count >= size of the type)
                    value <<= 8 * (sizeof(wchar_t)-1);
                    value <<= 8;  
                    value |= character & masks[sizeof(wchar_t)-1];
                }
            }
            else {
            // make sure no overflow will occur below
                if ((value & overflow_masks[sizeof(char)-1]) != 0) {
                    overflow |= true;
                }
                else {
                // calculate the new value
                    value <<= 8 * sizeof(char);
                    value |= character & masks[sizeof(char)-1];
                }
            }
        }
    };
    phoenix::function<compose_character_literal> const compose;

}   // namespace impl

///////////////////////////////////////////////////////////////////////////////
//  define, whether the rule's should generate some debug output
#define TRACE_CHLIT_GRAMMAR \
    bool(BOOST_SPIRIT_DEBUG_FLAGS_CPP & BOOST_SPIRIT_DEBUG_FLAGS_CHLIT_GRAMMAR) \
    /**/

struct chlit_grammar :
    public boost::spirit::grammar<chlit_grammar, 
        closures::chlit_closure::context_t>
{
    chlit_grammar()
    :   overflow(false)
    {
        BOOST_SPIRIT_DEBUG_TRACE_GRAMMAR_NAME(*this, "chlit_grammar", 
            TRACE_CHLIT_GRAMMAR);
    }
    
    template <typename ScannerT>
    struct definition
    {
        typedef 
            boost::spirit::rule<ScannerT, closures::chlit_closure::context_t> 
            rule_t;

        rule_t ch_lit;

        definition(chlit_grammar const &self)
        {
            using namespace boost::spirit;
            using namespace phoenix;
            
            // special parsers for '\x..' and L'\x....'
            typedef uint_parser<
                        unsigned int, 16, 1, 2 * sizeof(char)
                    > hex_char_parser_type;
            typedef uint_parser<
                        unsigned int, 16, 1, 2 * sizeof(wchar_t)
                    > hex_wchar_parser_type;

            // the rule for a character literal
            ch_lit
                =   eps_p[self.value = val(0), self.long_lit = val(false)]
                    >> !ch_p('L')[self.long_lit = val(true)]
                    >>  ch_p('\'')
                    >> +(   (
                            ch_p('\\') 
                            >>  (   ch_p('a')    // BEL
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x07))
                                    ]
                                |   ch_p('b')    // BS
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x08))
                                    ]
                                |   ch_p('t')    // HT
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x09))
                                    ]
                                |   ch_p('n')    // NL
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x0a))
                                    ]
                                |   ch_p('v')    // VT
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x0b))
                                    ]
                                |   ch_p('f')    // FF
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x0c))
                                    ]
                                |   ch_p('r')    // CR
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val(0x0d))
                                    ]
                                |   ch_p('?')
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val('?'))
                                    ]
                                |   ch_p('\'')
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val('\''))
                                    ]
                                |   ch_p('\"')
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val('\"'))
                                    ]
                                |   ch_p('\\')
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), val('\\'))
                                    ]
                                |   ch_p('x') 
                                    >>  if_p(self.long_lit) 
                                        [
                                            hex_wchar_parser_type()
                                            [
                                                impl::compose(self.value, self.long_lit, 
                                                    var(self.overflow), arg1)
                                            ]
                                        ]
                                        .else_p
                                        [
                                            hex_char_parser_type()
                                            [
                                                impl::compose(self.value, self.long_lit, 
                                                    var(self.overflow), arg1)
                                            ]
                                        ]
                                |   ch_p('u') 
                                    >>  uint_parser<unsigned int, 16, 4, 4>()
                                        [
                                            impl::compose(self.value, self.long_lit, 
                                                var(self.overflow), arg1)
                                        ]
                                |   ch_p('U')
                                    >>  uint_parser<unsigned int, 16, 8, 8>()
                                        [
                                            impl::compose(self.value, self.long_lit, 
                                                var(self.overflow), arg1)
                                        ]
                                |   uint_parser<unsigned int, 8, 1, 3>()
                                    [
                                        impl::compose(self.value, self.long_lit, 
                                            var(self.overflow), arg1)
                                    ]
                                )
                            )
                        |   ~eps_p(ch_p('\'')) >> anychar_p
                            [
                                impl::compose(self.value, self.long_lit, 
                                    var(self.overflow), arg1)
                            ]
                        )
                    >>  ch_p('\'')
                ;

            BOOST_SPIRIT_DEBUG_TRACE_RULE(ch_lit, TRACE_CHLIT_GRAMMAR);
        }

    // start rule of this grammar
        rule_t const& start() const
        { return ch_lit; }
    };

    // flag signaling integer overflow during value composition
    mutable bool overflow;
};

#undef TRACE_CHLIT_GRAMMAR

///////////////////////////////////////////////////////////////////////////////
//  
//  The following function is defined here, to allow the separation of 
//  the compilation of the intlit_grammap from the function using it.
//  
///////////////////////////////////////////////////////////////////////////////

#if BOOST_WAVE_SEPARATE_GRAMMAR_INSTANTIATION != 0
#define BOOST_WAVE_CHLITGRAMMAR_GEN_INLINE
#else
#define BOOST_WAVE_CHLITGRAMMAR_GEN_INLINE inline
#endif 

template <typename TokenT>
BOOST_WAVE_CHLITGRAMMAR_GEN_INLINE 
unsigned int
chlit_grammar_gen<TokenT>::evaluate(TokenT const &token)
{
    using namespace boost::spirit;
    
chlit_grammar g;
boost::uint32_t result = 0;
typename TokenT::string_type const &token_val = token.get_value();
parse_info<typename TokenT::string_type::const_iterator> hit =
    parse(token_val.begin(), token_val.end(), g[spirit_assign_actor(result)]);

    if (!hit.hit) {
        BOOST_WAVE_THROW(preprocess_exception, ill_formed_character_literal, 
            token_val.c_str(), token.get_position());
    }
    else {
    // range check
        if ('L' == token_val[0]) {
        // recognised wide character
            if (g.overflow || 
                result > (unsigned long)(std::numeric_limits<wchar_t>::max)()) 
            {
            // out of range
                BOOST_WAVE_THROW(preprocess_exception, 
                    character_literal_out_of_range, 
                    token_val.c_str(), token.get_position());
            }
        }
        else {
        // recognised narrow ('normal') character
            if (g.overflow || 
                result > (unsigned long)(std::numeric_limits<unsigned char>::max)()) 
            {
            // out of range
                BOOST_WAVE_THROW(preprocess_exception, 
                    character_literal_out_of_range, 
                    token_val.c_str(), token.get_position());
            }
        }
    }
    return result;
}

#undef BOOST_WAVE_CHLITGRAMMAR_GEN_INLINE

///////////////////////////////////////////////////////////////////////////////
}   // namespace grammars
}   // namespace wave
}   // namespace boost

#endif // !defined(CPP_CHLIT_GRAMMAR_HPP_9527D349_6592_449A_A409_42A001E6C64C_INCLUDED)
