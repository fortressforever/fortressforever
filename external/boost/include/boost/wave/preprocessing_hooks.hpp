/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(PREPROCESSING_HOOKS_HPP_338DE478_A13C_4B63_9BA9_041C917793B8_INCLUDED)
#define PREPROCESSING_HOOKS_HPP_338DE478_A13C_4B63_9BA9_041C917793B8_INCLUDED

#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace context_policies {

///////////////////////////////////////////////////////////////////////////////
//  
//  The default_preprocessing_hooks class is a placeholder for all 
//  preprocessing hooks called from inside the preprocessing engine
//
///////////////////////////////////////////////////////////////////////////////
struct default_preprocessing_hooks {

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'expanding_function_like_macro' is called, whenever a 
    //  function-like macro is to be expanded.
    //
    //  The macroname parameter marks the position, where the macro to expand 
    //  is defined.
    //  The formal_args parameter holds the formal arguments used during the
    //  definition of the macro.
    //  The definition parameter holds the macro definition for the macro to 
    //  trace.
    //
    //  The macro call parameter marks the position, where this macro invoked.
    //  The arguments parameter holds the macro arguments used during the 
    //  invocation of the macro
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenT, typename ContainerT>
    void expanding_function_like_macro(
        TokenT const &macrodef, std::vector<TokenT> const &formal_args, 
        ContainerT const &definition,
        TokenT const &macrocall, std::vector<ContainerT> const &arguments) 
    {}

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'expanding_object_like_macro' is called, whenever a 
    //  object-like macro is to be expanded .
    //
    //  The macroname parameter marks the position, where the macro to expand 
    //  is defined.
    //  The definition parameter holds the macro definition for the macro to 
    //  trace.
    //
    //  The macro call parameter marks the position, where this macro invoked.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenT, typename ContainerT>
    void expanding_object_like_macro(TokenT const &macro, 
        ContainerT const &definition, TokenT const &macrocall)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'expanded_macro' is called, whenever the expansion of a 
    //  macro is finished but before the rescanning process starts.
    //
    //  The parameter 'result' contains the token sequence generated as the 
    //  result of the macro expansion.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename ContainerT>
    void expanded_macro(ContainerT const &result)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'rescanned_macro' is called, whenever the rescanning of a 
    //  macro is finished.
    //
    //  The parameter 'result' contains the token sequence generated as the 
    //  result of the rescanning.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename ContainerT>
    void rescanned_macro(ContainerT const &result)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'found_include_directive' is called, whenever a #include
    //  directive was located.
    //
    //  The parameter 'filename' contains the (expanded) file name found after 
    //  the #include directive. This has the format '<file>', '"file"' or 
    //  'file'.
    //  The formats '<file>' or '"file"' are used for #include directives found 
    //  in the preprocessed token stream, the format 'file' is used for files
    //  specified through the --force_include command line argument.
    //
    //  The parameter 'include_next' is set to true if the found directive was
    //  a #include_next directive and the BOOST_WAVE_SUPPORT_INCLUDE_NEXT
    //  preprocessing constant was defined to something != 0.
    //
    ///////////////////////////////////////////////////////////////////////////
    void 
    found_include_directive(std::string const &filename, bool include_next) 
    {}
    
    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'opened_include_file' is called, whenever a file referred 
    //  by an #include directive was successfully located and opened.
    //
    //  The parameter 'filename' contains the file system path of the 
    //  opened file (this is relative to the directory of the currently 
    //  processed file or a absolute path depending on the paths given as the
    //  include search paths).
    //
    //  The include_depth parameter contains the current include file depth.
    //
    //  The is_system_include parameter denotes, whether the given file was 
    //  found as a result of a #include <...> directive.
    //  
    ///////////////////////////////////////////////////////////////////////////
    void 
    opened_include_file(std::string const &relname, std::string const &absname, 
        std::size_t include_depth, bool is_system_include) 
    {}
    
    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'returning_from_include_file' is called, whenever an
    //  included file is about to be closed after it's processing is complete.
    //
    ///////////////////////////////////////////////////////////////////////////
    void
    returning_from_include_file() 
    {}

    ///////////////////////////////////////////////////////////////////////////
    //  
    //  The function 'interpret_pragma' is called, whenever a #pragma wave 
    //  directive is found, which isn't known to the core Wave library. 
    //
    //  The parameter 'ctx' is a reference to the context object used for 
    //  instantiating the preprocessing iterators by the user.
    //
    //  The parameter 'pending' may be used to push tokens back into the input 
    //  stream, which are to be used as the replacement text for the whole 
    //  #pragma wave() directive.
    //
    //  The parameter 'option' contains the name of the interpreted pragma.
    //
    //  The parameter 'values' holds the values of the parameter provided to 
    //  the pragma operator.
    //
    //  The parameter 'act_token' contains the actual #pragma token, which may 
    //  be used for error output.
    //
    //  If the return value is 'false', the whole #pragma directive is 
    //  interpreted as unknown and a corresponding error message is issued. A
    //  return value of 'true' signs a successful interpretation of the given 
    //  #pragma.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename ContextT, typename ContainerT>
    bool 
    interpret_pragma(ContextT const &ctx, ContainerT &pending, 
        typename ContextT::token_type const &option, ContainerT const &values, 
        typename ContextT::token_type const &act_token)
    {
        return false;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  The function 'defined_macro' is called, whenever a macro was defined
    //  successfully.
    //
    //  The parameter 'name' is a reference to the token holding the macro name.
    //
    //  The parameter 'is_functionlike' is set to true, whenever the newly 
    //  defined macro is defined as a function like macro.
    //
    //  The parameter 'parameters' holds the parameter tokens for the macro
    //  definition. If the macro has no parameters or if it is a object like
    //  macro, then this container is empty.
    //
    //  The parameter 'definition' contains the token sequence given as the
    //  replacement sequence (definition part) of the newly defined macro.
    //
    //  The parameter 'is_predefined' is set to true for all macros predefined 
    //  during the initialisation phase of the library.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenT, typename ParametersT, typename DefinitionT>
    void
    defined_macro(TokenT const &macro_name, bool is_functionlike, 
        ParametersT const &parameters, DefinitionT const &definition, 
        bool is_predefined)
    {}
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  The function 'undefined_macro' is called, whenever a macro definition
    //  was removed successfully.
    //  
    //  The parameter 'name' holds the name of the macro, which definition was 
    //  removed.
    //
    ///////////////////////////////////////////////////////////////////////////
    template <typename TokenT>
    void
    undefined_macro(TokenT const &macro_name)
    {}
    
};

///////////////////////////////////////////////////////////////////////////////
}   // namespace context_policies
}   // namespace wave
}   // namespace boost

#endif // !defined(PREPROCESSING_HOOKS_HPP_338DE478_A13C_4B63_9BA9_041C917793B8_INCLUDED)
