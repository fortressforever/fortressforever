/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(ITERATION_CONTEXT_HPP_9556CD16_F11E_4ADC_AC8B_FB9A174BE664_INCLUDED)
#define ITERATION_CONTEXT_HPP_9556CD16_F11E_4ADC_AC8B_FB9A174BE664_INCLUDED

#include <cstdlib>
#include <cstdio>
#include <stack>

#include <boost/wave/wave_config.hpp>
#include <boost/wave/cpp_exceptions.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace util {

///////////////////////////////////////////////////////////////////////////////
template <typename IterationContextT>
class iteration_context_stack 
{
    typedef std::stack<IterationContextT> base_type;
    
public:
    typedef typename base_type::size_type size_type;
    
    iteration_context_stack()
    :   max_include_nesting_depth(BOOST_WAVE_MAX_INCLUDE_LEVEL_DEPTH)
    {}
    
    void set_max_include_nesting_depth(size_type new_depth)
        {  max_include_nesting_depth = new_depth; }
    size_type get_max_include_nesting_depth() const
        { return max_include_nesting_depth; }

    typename base_type::size_type size() const { return iter_ctx.size(); }
    typename base_type::value_type &top() { return iter_ctx.top(); }
    void pop() { iter_ctx.pop(); }
    
    template <typename PositionT>
    void push(PositionT const &pos, typename base_type::value_type const &val)
    { 
        if (iter_ctx.size() == max_include_nesting_depth) {
        char buffer[22];    // 21 bytes holds all NUL-terminated unsigned 64-bit numbers

            using namespace std;    // for some systems ltoa is in namespace std
            sprintf(buffer, "%d", (int)max_include_nesting_depth);
            BOOST_WAVE_THROW(preprocess_exception, include_nesting_too_deep, 
                buffer, pos);
        }
        iter_ctx.push(val); 
    }
        
private:
    size_type max_include_nesting_depth;
    base_type iter_ctx;
};

///////////////////////////////////////////////////////////////////////////////
}   // namespace util
}   // namespace wave
}   // namespace boost

#endif // !defined(ITERATION_CONTEXT_HPP_9556CD16_F11E_4ADC_AC8B_FB9A174BE664_INCLUDED)
