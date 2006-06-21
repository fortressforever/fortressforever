//
// Boost.Pointer Container
//
//  Copyright Thorsten Ottosen 2003-2005. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/ptr_container/
//

#ifndef BOOST_PTR_CONTAINER_PTR_ARRAY_HPP
#define BOOST_PTR_CONTAINER_PTR_ARRAY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <boost/array.hpp>
#include <boost/static_assert.hpp>
#include <boost/ptr_container/ptr_sequence_adapter.hpp>

namespace boost
{

    namespace ptr_container_detail
    {
        template
        <
            class T, 
            size_t N,
            class Allocator = int // dummy
        >
        class ptr_array_impl : public boost::array<T,N>
        {
        public:
            typedef Allocator allocator_type;
            
            ptr_array_impl( Allocator a = Allocator() ) 
            {
                this->assign( 0 ); 
            }
            
            ptr_array_impl( size_t, T*, Allocator a = Allocator() )
            {
                this->assing( 0 );
            }
        };
    }
    
    template
    < 
        class T, 
        size_t N, 
        class CloneAllocator = heap_clone_allocator
    >
    class ptr_array : public 
        ptr_sequence_adapter< T, 
                              ptr_container_detail::ptr_array_impl<void*,N>,
                              CloneAllocator >
    {  
    private:
        typedef ptr_sequence_adapter< T,   
                                      ptr_container_detail::ptr_array_impl<void*,N>,
                                      CloneAllocator >
            base_class;

        typedef BOOST_DEDUCED_TYPENAME remove_nullable<T>::type U;

        typedef ptr_array<T,N,CloneAllocator> 
                          this_type;

        ptr_array( const this_type& );
        void operator=( const this_type& );
        
    public:
        typedef U*        value_type;
        typedef U*        pointer;
        typedef U&        reference;
        typedef const U&  const_reference;
        typedef BOOST_DEDUCED_TYPENAME base_class::auto_type 
                          auto_type;
        
    public: // constructors
        ptr_array() : base_class()
        { }
        
        ptr_array( std::auto_ptr<this_type> r ) 
        : base_class( r ) { }            

        void operator=( std::auto_ptr<this_type> r )
        {
            base_class::operator=(r);
        }
        
        std::auto_ptr<this_type> release()          
        {                                    
            std::auto_ptr<this_type> ptr( new this_type );     
            this->swap( *ptr );                  
            return ptr;                          
        }                                    
                
        std::auto_ptr<this_type> clone() const      
        {                                    
            std::auto_ptr<this_type> pa( new this_type );
            for( size_t i = 0; i != N; ++i )
            {
                if( ! is_null(i) )
                    pa->replace( i, CloneAllocator::allocate_clone( (*this)[i] ) );
            }
            return pa;
        }
        
    private: // hide some members
        using base_class::insert;
        using base_class::erase;
        using base_class::push_back;
        using base_class::push_front;
        using base_class::pop_front;
        using base_class::pop_back;
        using base_class::transfer;
        using base_class::get_allocator;
                
    public: // compile-time interface

        template< size_t idx >
        auto_type replace( U* r ) // strong
        {
            BOOST_STATIC_ASSERT( idx < N );

            this->enforce_null_policy( r, "Null pointer in 'ptr_array::replace()'" );
            
            auto_type res( static_cast<U*>( this->c_private()[idx] ) ); // nothrow
            this->c_private()[idx] = r;                                      // nothrow
            return move(res);                                                // nothrow
        }

        auto_type replace( size_t idx, U* r ) // strong
        {
            this->enforce_null_policy( r, "Null pointer in 'ptr_array::replace()'" );

            auto_type ptr( r );

            if( idx >= N )
                throw bad_index( "'replace()' aout of bounds" );

            auto_type res( static_cast<U*>( this->c_private()[idx] ) ); // nothrow
            this->c_private()[idx] = ptr.release();                          // nothrow
            return move(res);                                                // nothrow
        }

        using base_class::at;

        template< size_t idx >
        T& at()
        {
            BOOST_STATIC_ASSERT( idx < N );
            return (*this)[idx]; 
        }

        template< size_t idx >
        const T& at() const
        {
            BOOST_STATIC_ASSERT( idx < N );
            return (*this)[idx]; 
        }

        bool is_null( size_t idx ) const
        {
            return base_class::is_null(idx);
        }
        
        template< size_t idx >
        bool is_null() const
        {
            BOOST_STATIC_ASSERT( idx < N );
            return this->c_private()[idx] == 0;
        }

    };

    //////////////////////////////////////////////////////////////////////////////
    // clonability

    template< typename T, size_t size, typename CA >
    inline ptr_array<T,size,CA>* new_clone( const ptr_array<T,size,CA>& r )
    {
        return r.clone().release();
    }

    /////////////////////////////////////////////////////////////////////////
    // swap

    template< typename T, size_t size, typename CA >
    inline void swap( ptr_array<T,size,CA>& l, ptr_array<T,size,CA>& r )
    {
        l.swap(r);
    }
}

#endif
