/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    Detect the need to insert a whitespace token into the output stream
    
    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(INSERT_WHITESPACE_DETECTION_HPP_765EF77B_0513_4967_BDD6_6A38148C4C96_INCLUDED)
#define INSERT_WHITESPACE_DETECTION_HPP_765EF77B_0513_4967_BDD6_6A38148C4C96_INCLUDED

#include <boost/wave/wave_config.hpp>   
#include <boost/wave/token_ids.hpp>   

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace util {

namespace impl {

// T_IDENTIFIER
    template <typename StringT>
    inline bool
    would_form_universal_char (StringT const &value)
    {
        if ('u' != value[0] && 'U' != value[0])
            return false;
        if ('u' == value[0] && value.size() < 5)
            return false;
        if ('U' == value[0] && value.size() < 9)
            return false;
    
    typename StringT::size_type pos = 
        value.find_first_not_of("0123456789abcdefABCDEF", 1);
        
        if (StringT::npos == pos || 
            ('u' == value[0] && pos > 5) ||
            ('U' == value[0] && pos > 9))
        {
            return true;        // would form an universal char
        }
        return false;
    }
    template <typename StringT>
    inline bool 
    handle_identifier(boost::wave::token_id prev, 
        boost::wave::token_id before, StringT const &value)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_IDENTIFIER:
        case T_NONREPLACABLE_IDENTIFIER:
        case T_COMPL_ALT:
        case T_OR_ALT:
        case T_AND_ALT:
        case T_NOT_ALT:
        case T_XOR_ALT:
        case T_ANDASSIGN_ALT:
        case T_ORASSIGN_ALT:
        case T_XORASSIGN_ALT:
        case T_NOTEQUAL_ALT:
        case T_FIXEDPOINTLIT:
            return true;

        case T_FLOATLIT:
        case T_INTLIT:
            return (value.size() > 1 || (value[0] != 'e' && value[0] != 'E'));
            
         // avoid constructing universal characters (\u1234)
        case TOKEN_FROM_ID('\\', UnknownTokenType):
            return would_form_universal_char(value);
        }
        return false;
    }
// T_INTLIT
    inline bool 
    handle_intlit(boost::wave::token_id prev, boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_IDENTIFIER:
        case T_NONREPLACABLE_IDENTIFIER:
        case T_INTLIT:
        case T_FLOATLIT:
        case T_FIXEDPOINTLIT:
            return true;
        }
        return false;
    }
// T_FLOATLIT
    inline bool 
    handle_floatlit(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_IDENTIFIER:
        case T_NONREPLACABLE_IDENTIFIER:
        case T_INTLIT:
        case T_FLOATLIT:
        case T_FIXEDPOINTLIT:
            return true;
        }
        return false;
    }
// <% T_LEFTBRACE
    inline bool 
    handle_alt_leftbrace(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_LESS:        // <<%
        case T_SHIFTLEFT:   // <<<%
            return true;
        }
        return false;
    }
// <: T_LEFTBRACKET
    inline bool 
    handle_alt_leftbracket(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_LESS:        // <<:
        case T_SHIFTLEFT:   // <<<:
            return true;
        }
        return false;
    }
// T_FIXEDPOINTLIT
    inline bool 
    handle_fixedpointlit(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_IDENTIFIER:
        case T_NONREPLACABLE_IDENTIFIER:
        case T_INTLIT:
        case T_FLOATLIT:
        case T_FIXEDPOINTLIT:
            return true;
        }
        return false;
    }
// T_DOT
    inline bool 
    handle_dot(boost::wave::token_id prev, boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch (static_cast<unsigned int>(prev)) {
        case T_DOT:
            if (T_DOT == before)
                return true;    // ...
            break;
        }
        return false;
    }
// T_QUESTION_MARK
    inline bool 
    handle_questionmark(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch(static_cast<unsigned int>(prev)) {
        case TOKEN_FROM_ID('\\', UnknownTokenType):     // \?
        case T_QUESTION_MARK:   // ??
            return true;
        }
        return false;
    }
// T_NEWLINE
    inline bool
    handle_newline(boost::wave::token_id prev, 
        boost::wave::token_id before)
    {
        using namespace boost::wave;
        switch(static_cast<unsigned int>(prev)) {
        case TOKEN_FROM_ID('\\', UnknownTokenType): // \ \n
        case T_DIVIDE:
            if (T_QUESTION_MARK == before)
                return true;    // ?/\n     // may be \\n
            break;
        }
        return false;
    }
    
}   // namespace impl

class insert_whitespace_detection 
{
public:
    insert_whitespace_detection() 
    :   prev(boost::wave::T_EOF), beforeprev(boost::wave::T_EOF) 
    {}
    
    template <typename StringT>
    bool must_insert(boost::wave::token_id current, StringT const &value)
    {
        using namespace boost::wave;
        switch (current) {
        case T_NONREPLACABLE_IDENTIFIER:
        case T_IDENTIFIER: 
            return impl::handle_identifier(prev, beforeprev, value); 
        case T_INTLIT:
            return impl::handle_intlit(prev, beforeprev); 
        case T_FLOATLIT:
            return impl::handle_floatlit(prev, beforeprev); 
        case T_STRINGLIT:
            if (TOKEN_FROM_ID('L', UnknownTokenType) == prev)       // 'L'
                return true;
            break;
        case T_LEFTBRACE_ALT:
            return impl::handle_alt_leftbrace(prev, beforeprev); 
        case T_LEFTBRACKET_ALT:
            return impl::handle_alt_leftbracket(prev, beforeprev); 
        case T_FIXEDPOINTLIT:
            return impl::handle_fixedpointlit(prev, beforeprev); 
        case T_DOT:
            return impl::handle_dot(prev, beforeprev); 
        case T_QUESTION_MARK:
            return impl::handle_questionmark(prev, beforeprev); 
        case T_NEWLINE:
            return impl::handle_newline(prev, beforeprev); 

        case T_LEFTPAREN:
        case T_RIGHTPAREN:
        case T_LEFTBRACKET:
        case T_RIGHTBRACKET:
        case T_SEMICOLON:
        case T_COMMA:
        case T_COLON:
            switch (static_cast<unsigned int>(prev)) {
            case T_LEFTPAREN:
            case T_RIGHTPAREN:
            case T_LEFTBRACKET:
            case T_RIGHTBRACKET:
            case T_LEFTBRACE:
            case T_RIGHTBRACE:
                return false;   // no insertion between parens/brackets/braces

            default:
                break;
            }        
            break;
            
        case T_LEFTBRACE:
        case T_RIGHTBRACE:
            switch (static_cast<unsigned int>(prev)) {
            case T_LEFTPAREN:
            case T_RIGHTPAREN:
            case T_LEFTBRACKET:
            case T_RIGHTBRACKET:
            case T_LEFTBRACE:
            case T_RIGHTBRACE:
            case T_SEMICOLON:
            case T_COMMA:
            case T_COLON:
                return false;   // no insertion between parens/brackets/braces

            case T_QUESTION_MARK:
                if (T_QUESTION_MARK == beforeprev)
                    return true;
                break;
                
            default:
                break;
            }
            break;
                            
        case T_MINUS:
        case T_MINUSMINUS:
        case T_LESS:
        case T_EQUAL:
        case T_ASSIGN:
        case T_GREATER:
        case T_DIVIDE:
        case T_CHARLIT:
        case T_NOT:
        case T_NOTEQUAL:
        case T_DIVIDEASSIGN:
        case T_MINUSASSIGN:
            if (T_QUESTION_MARK == prev && T_QUESTION_MARK == beforeprev)
                return true;    // ??{op}
            break;

        case T_COMPL_ALT:
        case T_OR_ALT:
        case T_AND_ALT:
        case T_NOT_ALT:
        case T_XOR_ALT:
        case T_ANDASSIGN_ALT:
        case T_ORASSIGN_ALT:
        case T_XORASSIGN_ALT:
        case T_NOTEQUAL_ALT:
            if (T_IDENTIFIER == prev || T_NONREPLACABLE_IDENTIFIER == prev ||
                IS_CATEGORY(prev, KeywordTokenType))
                return true;
            break;
        }

    // else, handle operators separately
        if (IS_CATEGORY(current, OperatorTokenType) && 
            IS_CATEGORY(prev, OperatorTokenType))
        {
            return true;    // operators must be delimited always
        }
        return false;
    }
    void shift_tokens (boost::wave::token_id next_id)
    {
        beforeprev = prev;
        prev = next_id;
    }
    
private:
    boost::wave::token_id prev;        // the previous analyzed token
    boost::wave::token_id beforeprev;  // the token before the previous
};

///////////////////////////////////////////////////////////////////////////////
}   //  namespace util
}   //  namespace wave 
}   //  namespace boost

#endif // !defined(INSERT_WHITESPACE_DETECTION_HPP_765EF77B_0513_4967_BDD6_6A38148C4C96_INCLUDED)
