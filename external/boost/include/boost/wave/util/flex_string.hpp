/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library
    http://www.boost.org/

    Copyright (c) 2001 by Andrei Alexandrescu. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// This code is taken from: 
// Andrei Alexandrescu, Generic<Programming>: A Policy-Based basic_string 
// Implementation. http://www.cuj.com/documents/s=7994/cujcexp1906alexandr/
//
// #HK030306: 
//      - Moved into the namespace boost::wave::util 
//      - Added a bunch of missing typename(s) 
//      - Integrated with boost config
//      - Added a missing header include
//      - Added special constructors and operator= to allow CowString to be 
//        a real COW-string (removed unnecessary data copying)
//      - Fixed a string terminating bug in append
//
// #HK040109:
//      - Incorporated the changes from Andrei's latest version of this class

#ifndef FLEX_STRING_INC_
#define FLEX_STRING_INC_

/*
////////////////////////////////////////////////////////////////////////////////
template <typename E, class A = @>
class StoragePolicy
{
    typedef E value_type;
    typedef @ iterator;
    typedef @ const_iterator;
    typedef A allocator_type;
    typedef @ size_type;
    
    StoragePolicy(const StoragePolicy& s);
    StoragePolicy(const A&);
    StoragePolicy(const E* s, size_type len, const A&);
    StoragePolicy(size_type len, E c, const A&);
    ~StoragePolicy();

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    
    size_type size() const;
    size_type max_size() const;
    size_type capacity() const;

    void reserve(size_type res_arg);

    void append(const E* s, size_type sz);
    
    template <class InputIterator>
    void append(InputIterator b, InputIterator e);

    void resize(size_type newSize, E fill);

    void swap(StoragePolicy& rhs);
    
    const E* c_str() const;
    const E* data() const;
    
    A get_allocator() const;
};
////////////////////////////////////////////////////////////////////////////////
*/

#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/iterator/reverse_iterator.hpp>

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <cstddef>

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace util {

namespace flex_string_details
{
    template <class Pod, class T>
    inline void pod_fill(Pod* b, Pod* e, T c)
    {
        switch ((e - b) & 7)
        {
        case 0:
            while (b != e)
            {
                *b = c; ++b;
        case 7: *b = c; ++b;
        case 6: *b = c; ++b;
        case 5: *b = c; ++b;
        case 4: *b = c; ++b;
        case 3: *b = c; ++b;
        case 2: *b = c; ++b;
        case 1: *b = c; ++b;
            }
        }
    }

    template <class Pod>
    inline void pod_move(const Pod* b, const Pod* e, Pod* d)
    {
        using namespace std;
        memmove(d, b, (e - b) * sizeof(*b));
    }

    template <class Pod>
    inline Pod* pod_copy(const Pod* b, const Pod* e, Pod* d)
    {
        const std::size_t s = e - b;
        using namespace std;
        memcpy(d, b, s * sizeof(*b));
        return d + s;
    }

    template <typename T> struct get_unsigned
    {
        typedef T result;
    };

    template <> struct get_unsigned<char>
    {
        typedef unsigned char result;
    };

    template <> struct get_unsigned<signed char>
    {
        typedef unsigned char result;
    };

    template <> struct get_unsigned<short int>
    {
        typedef unsigned short int result;
    };

    template <> struct get_unsigned<int>
    {
        typedef unsigned int result;
    };

    template <> struct get_unsigned<long int>
    {
        typedef unsigned long int result;
    };

    enum Shallow {};
}

template <class T> class mallocator
{
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    //typedef unsigned int      size_type;
    //typedef std::ptrdiff_t    difference_type;
    typedef int               difference_type;

    template <class U> 
    struct rebind { typedef mallocator<U> other; };

    mallocator() {}
    mallocator(const mallocator&) {}
    //template <class U> 
    //mallocator(const mallocator<U>&) {}
    ~mallocator() {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const 
    { 
        return x;
    }

    pointer allocate(size_type n, const_pointer = 0) 
    {
        using namespace std;
        void* p = malloc(n * sizeof(T));
        if (!p) throw bad_alloc();
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type) 
    { 
        using namespace std;
        free(p); 
    }

    size_type max_size() const 
    { 
        return static_cast<size_type>(-1) / sizeof(T);
    }

    void construct(pointer p, const value_type& x) 
    { 
        new(p) value_type(x); 
    }

    void destroy(pointer p) 
    { 
        p->~value_type(); 
    }

private:
    void operator=(const mallocator&);
};

template<> class mallocator<void>
{
  typedef void        value_type;
  typedef void*       pointer;
  typedef const void* const_pointer;

  template <class U> 
  struct rebind { typedef mallocator<U> other; };
};

template <class T>
inline bool operator==(const mallocator<T>&, 
                       const mallocator<T>&) {
  return true;
}

template <class T>
inline bool operator!=(const mallocator<T>&, 
                       const mallocator<T>&) {
  return false;
}

template <class Allocator>
typename Allocator::pointer Reallocate(
    Allocator& alloc,
    typename Allocator::pointer p, 
    typename Allocator::size_type oldObjCount,
    typename Allocator::size_type newObjCount,
    void*)
{
    // @@@ not implemented
    return NULL;
}

template <class Allocator>
typename Allocator::pointer Reallocate(
    Allocator& alloc,
    typename Allocator::pointer p, 
    typename Allocator::size_type oldObjCount,
    typename Allocator::size_type newObjCount,
    mallocator<void>*)
{
    // @@@ not implemented
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// class template SimpleStringStorage
// Allocates memory with malloc
////////////////////////////////////////////////////////////////////////////////

template <typename E, class A = std::allocator<E> >
class SimpleStringStorage
{
    // The "public" below exists because MSVC can't do template typedefs
public:
    struct Data
    {
        Data() : pEnd_(buffer_), pEndOfMem_(buffer_) { buffer_[0] = E(0); }

        E* pEnd_;
        E* pEndOfMem_;
        E buffer_[1];
    };
    static const Data emptyString_;
    
    typedef typename A::size_type size_type;

private:
    Data* pData_;

    void Init(size_type size, size_type capacity)
    {
        BOOST_ASSERT(size <= capacity);
        if (capacity == 0) 
        {
            pData_ = const_cast<Data*>(&emptyString_);
        }
        else
        {
            // 11-17-2000: comment added: 
            //     No need to allocate (capacity + 1) to 
            //     accomodate the terminating 0, because Data already
            //     has one one character in there
            pData_ = static_cast<Data*>(
                malloc(sizeof(Data) + capacity * sizeof(E)));
            if (!pData_) throw std::bad_alloc();
            pData_->pEnd_ = pData_->buffer_ + size;
            pData_->pEndOfMem_ = pData_->buffer_ + capacity;
        }
    }
    
private:
    // Warning - this doesn't initialize pData_. Used in reserve()
    SimpleStringStorage()
    { }
    
public:
    typedef E value_type;
    typedef E* iterator;
    typedef const E* const_iterator;
    typedef A allocator_type;
    
    SimpleStringStorage(const SimpleStringStorage& rhs) 
    {
        const size_type sz = rhs.size();
        Init(sz, sz);
        if (sz) flex_string_details::pod_copy(rhs.begin(), rhs.end(), begin());
    }
    
    SimpleStringStorage(const SimpleStringStorage& s, 
        flex_string_details::Shallow) 
        : pData_(s.pData_)
    {
    }
    
    SimpleStringStorage(const A&)
    { pData_ = const_cast<Data*>(&emptyString_); }
    
    SimpleStringStorage(const E* s, size_type len, const A&)
    {
        Init(len, len);
        flex_string_details::pod_copy(s, s + len, begin());
    }

    SimpleStringStorage(size_type len, E c, const A&)
    {
        Init(len, len);
        flex_string_details::pod_fill(begin(), end(), c);
    }
    
    SimpleStringStorage& operator=(const SimpleStringStorage& rhs)
    {
        const size_type sz = rhs.size();
        reserve(sz);
        flex_string_details::pod_copy(&*rhs.begin(), &*rhs.end(), begin());
        pData_->pEnd_ = &*begin() + sz;
        return *this;
    }

    ~SimpleStringStorage()
    {
        BOOST_ASSERT(begin() <= end());
        if (pData_ != &emptyString_) free(pData_);
    }

    iterator begin()
    { return pData_->buffer_; }
    
    const_iterator begin() const
    { return pData_->buffer_; }
    
    iterator end()
    { return pData_->pEnd_; }
    
    const_iterator end() const
    { return pData_->pEnd_; }
    
    size_type size() const
    { return pData_->pEnd_ - pData_->buffer_; }

    size_type max_size() const
    { return std::size_t(-1) / sizeof(E) - sizeof(Data) - 1; }

    size_type capacity() const
    { return pData_->pEndOfMem_ - pData_->buffer_; }

    void reserve(size_type res_arg)
    {
        if (res_arg <= capacity())
        {
            // @@@ insert shrinkage here if you wish
            return;
        }

        if (pData_ == &emptyString_) 
        {
            Init(0, res_arg);
        }
        else
        {
            const size_type sz = size();

            void* p = realloc(pData_, 
                sizeof(Data) + res_arg * sizeof(E));
            if (!p) throw std::bad_alloc();
        
            if (p != pData_)
            {
                pData_ = static_cast<Data*>(p);
                pData_->pEnd_ = pData_->buffer_ + sz;
            }
            pData_->pEndOfMem_ = pData_->buffer_ + res_arg;
        }
    }

    void append(const E* s, size_type sz)
    {
        const size_type neededCapacity = size() + sz;

        if (capacity() < neededCapacity)
        {
            const iterator b = begin();
            static std::less_equal<const E*> le;
            if (le(b, s) && le(s, end()))
            {
               // aliased
                const size_type offset = s - b;
                reserve(neededCapacity);
                s = begin() + offset;
            }
            else
            {
                reserve(neededCapacity);
            }
        }
        flex_string_details::pod_copy(s, s + sz, end());
        pData_->pEnd_ += sz;
    }
    
    template <class InputIterator>
    void append(InputIterator b, InputIterator e)
    {
        // @@@ todo: optimize this depending on iterator type
        for (; b != e; ++b)
        {
            *this += *b;
        }
    }

    void resize(size_type newSize, E fill)
    {
        const int delta = int(newSize - size());
        if (delta == 0) return;

        if (delta > 0)
        {
            if (newSize > capacity()) 
            {
                reserve(newSize);
            }
            E* e = &*end();
            flex_string_details::pod_fill(e, e + delta, fill);
        }
        pData_->pEnd_ = pData_->buffer_ + newSize;
    }

    void swap(SimpleStringStorage& rhs)
    {
        std::swap(pData_, rhs.pData_);
    }
    
    const E* c_str() const
    {
        if (pData_ != &emptyString_) *pData_->pEnd_ = E();
        return pData_->buffer_; 
    }

    const E* data() const
    { return pData_->buffer_; }
    
    A get_allocator() const
    { return A(); }
};

template <typename E, class A>
const typename SimpleStringStorage<E, A>::Data
SimpleStringStorage<E, A>::emptyString_ = typename SimpleStringStorage<E, A>::Data();
//{ 
//  const_cast<E*>(SimpleStringStorage<E, A>::emptyString_.buffer_), 
//  const_cast<E*>(SimpleStringStorage<E, A>::emptyString_.buffer_), 
//  { E() }
//};

////////////////////////////////////////////////////////////////////////////////
// class template AllocatorStringStorage
// Allocates with your allocator
// Takes advantage of the Empty Base Optimization if available
////////////////////////////////////////////////////////////////////////////////

template <typename E, class A = std::allocator<E> >
class AllocatorStringStorage : public A
{
    typedef typename A::size_type size_type;
    typedef typename SimpleStringStorage<E, A>::Data Data;

    void* Alloc(size_type sz, const void* p = 0)
    {
        return A::allocate(1 + (sz - 1) / sizeof(E), 
            static_cast<const char*>(p));
    }

    void* Realloc(void* p, size_type oldSz, size_type newSz)
    {
        void* r = Alloc(newSz);
        flex_string_details::pod_copy(p, p + Min(oldSz, newSz), r);
        Free(p, oldSz);
        return r;
    }

    void Free(void* p, size_type sz)
    {
        A::deallocate(static_cast<E*>(p), sz);
    }

    Data* pData_;

    void Init(size_type size, size_type cap)
    {
        BOOST_ASSERT(size <= cap);

        if (cap == 0)
        {
            pData_ = const_cast<Data*>(
                &SimpleStringStorage<E, A>::emptyString_);
        }
        else
        {
            pData_ = static_cast<Data*>(Alloc(
                cap * sizeof(E) + sizeof(Data)));
            pData_->pEnd_ = pData_->buffer_ + size;
            pData_->pEndOfMem_ = pData_->buffer_ + cap;
        }
    }
    
public:
    typedef E value_type;
    typedef A allocator_type;
    typedef typename A::pointer iterator;
    typedef typename A::const_pointer const_iterator;
    
    AllocatorStringStorage(const AllocatorStringStorage& rhs) 
    : A(rhs.get_allocator())
    {
        const size_type sz = rhs.size();
        Init(sz, sz);
        if (sz) flex_string_details::pod_copy(rhs.begin(), rhs.end(), begin());
    }
    
    AllocatorStringStorage(const AllocatorStringStorage& s, 
        flex_string_details::Shallow) 
    : A(s.get_allocator())
    {
        pData_ = s.pData_;
    }
    
    AllocatorStringStorage(const A& a) : A(a)
    { 
        pData_ = const_cast<Data*>(
            &SimpleStringStorage<E, A>::emptyString_);
    }
    
    AllocatorStringStorage(const E* s, size_type len, const A& a)
    : A(a)
    {
        Init(len, len);        
        flex_string_details::pod_copy(s, s + len, begin());
    }

    AllocatorStringStorage(size_type len, E c, const A& a)
    : A(a)
    {
        Init(len, len);
        flex_string_details::pod_fill(&*begin(), &*end(), c);
    }
    
    AllocatorStringStorage& operator=(const AllocatorStringStorage& rhs)
    {
        const size_type sz = rhs.size();
        reserve(sz);
        flex_string_details::pod_copy(&*rhs.begin(), &*rhs.end(), begin());
        pData_->pEnd_ = &*begin() + rhs.size();
        return *this;
    }
    
    ~AllocatorStringStorage()
    {
        if (capacity())
        {
            Free(pData_, 
                sizeof(Data) + capacity() * sizeof(E));
        }
    }
        
    iterator begin()
    { return pData_->buffer_; }
    
    const_iterator begin() const
    { return pData_->buffer_; }
    
    iterator end()
    { return pData_->pEnd_; }
    
    const_iterator end() const
    { return pData_->pEnd_; }
    
    size_type size() const
    { return size_type(end() - begin()); }

    size_type max_size() const
    { return A::max_size(); }

    size_type capacity() const
    { return size_type(pData_->pEndOfMem_ - pData_->buffer_); }

    void resize(size_type n, E c)
    {
        reserve(n);
        iterator newEnd = begin() + n;
        iterator oldEnd = end();
        if (newEnd > oldEnd) 
        {
            // Copy the characters
            flex_string_details::pod_fill(oldEnd, newEnd, c);
        }
        if (capacity()) pData_->pEnd_ = newEnd;
    }

    void reserve(size_type res_arg)
    {
        if (res_arg <= capacity())
        {
            // @@@ shrink to fit here 
            return;
        }
        
        A& myAlloc = *this;
        AllocatorStringStorage newStr(myAlloc);
        newStr.Init(size(), res_arg);
        
        flex_string_details::pod_copy(begin(), end(), newStr.begin());
        
        swap(newStr);
    }

    void append(const E* s, size_type sz)
    {
        const size_type neededCapacity = size() + sz;

        if (capacity() < neededCapacity)
        {
            const iterator b = begin();
            static std::less_equal<const E*> le;
            if (le(b, s) && le(s, end()))
            {
               // aliased
                const size_type offset = s - b;
                reserve(neededCapacity);
                s = begin() + offset;
            }
            else
            {
                reserve(neededCapacity);
            }
        }
        flex_string_details::pod_copy(s, s + sz, end());
        pData_->pEnd_ += sz;
    }
    
    void swap(AllocatorStringStorage& rhs)
    {
        // @@@ The following line is commented due to a bug in MSVC
        //std::swap(lhsAlloc, rhsAlloc);
        std::swap(pData_, rhs.pData_);
    }
    
    const E* c_str() const
    { 
        if (pData_ != &SimpleStringStorage<E, A>::emptyString_) 
        {
            *pData_->pEnd_ = E();
        }
        return &*begin(); 
    }

    const E* data() const
    { return &*begin(); }
    
    A get_allocator() const
    { return *this; }
};

////////////////////////////////////////////////////////////////////////////////
// class template VectorStringStorage
// Uses std::vector
// Takes advantage of the Empty Base Optimization if available
////////////////////////////////////////////////////////////////////////////////

template <typename E, class A = std::allocator<E> >
class VectorStringStorage : protected std::vector<E, A>
{
    typedef std::vector<E, A> base;

public: // protected:
    typedef E value_type;
    typedef typename base::iterator iterator;
    typedef typename base::const_iterator const_iterator;
    typedef A allocator_type;
    typedef typename A::size_type size_type;
    
    VectorStringStorage(const VectorStringStorage& s) : base(s)
    { }
    
    VectorStringStorage(const A& a) : base(1, E(), a)
    { }
    
    VectorStringStorage(const E* s, size_type len, const A& a)
    : base(a)
    {
        base::reserve(len + 1);
        base::insert(base::end(), s, s + len);
        // Terminating zero
        base::insert(base::end(), E());
    }

    VectorStringStorage(size_type len, E c, const A& a)
    : base(len + 1, c, a)
    {
        // Terminating zero
        base::back() = E();
    }
    
    VectorStringStorage& operator=(const VectorStringStorage& rhs)
    {
        base& v = *this;
        v = rhs;
        return *this;
    }
   
    iterator begin()
    { return base::begin(); }
    
    const_iterator begin() const
    { return base::begin(); }
    
    iterator end()
    { return base::end() - 1; }
    
    const_iterator end() const
    { return base::end() - 1; }
    
    size_type size() const
    { return base::size() - 1; }

    size_type max_size() const
    { return base::max_size() - 1; }

    size_type capacity() const
    { return base::capacity() - 1; }

    void reserve(size_type res_arg)
    { 
        BOOST_ASSERT(res_arg < max_size());
        base::reserve(res_arg + 1); 
    }
    
    void append(const E* s, size_type sz)
    {
        // Check for aliasing because std::vector doesn't do it.
        static std::less_equal<const E*> le;
        if (!base::empty())
        {
            const E* start = &base::front();
            if (le(start, s) && le(s, start + size()))
            {
                // aliased
                const size_type offset = s - start;
                reserve(size() + sz);
                s = &base::front() + offset;
            }
        }
        base::insert(end(), s, s + sz);
    }
    
    template <class InputIterator>
    void append(InputIterator b, InputIterator e)
    {
        base::insert(end(), b, e);
    }

    void resize(size_type n, E c)
    {
        base::reserve(n + 1);
        base::back() = c;
        base::resize(n + 1, c);
        base::back() = E();
    }

    void swap(VectorStringStorage& rhs)
    { base::swap(rhs); }
    
    const E* c_str() const
    { return &*begin(); }

    const E* data() const
    { return &*begin(); }
    
    A get_allocator() const
    { return base::get_allocator(); }
};

////////////////////////////////////////////////////////////////////////////////
// class template SmallStringOpt
// Builds the small string optimization over any other storage
////////////////////////////////////////////////////////////////////////////////

template <class Storage, unsigned int threshold, 
    typename Align = typename Storage::value_type*>
class SmallStringOpt
{
public:
    typedef typename Storage::value_type value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef typename Storage::allocator_type allocator_type;
    typedef typename allocator_type::size_type size_type;
    
private:
  enum { temp1 = threshold * sizeof(value_type) > sizeof(Storage) 
        ? threshold  * sizeof(value_type) 
        : sizeof(Storage) };
    
    enum { temp2 = temp1 > sizeof(Align) ? temp1 : sizeof(Align) };

public:
    enum { maxSmallString = 
    (temp2 + sizeof(value_type) - 1) / sizeof(value_type) };
    
private:
    enum { magic = maxSmallString + 1 };
    
    union
    {
        mutable value_type buf_[maxSmallString + 1];
        Align align_;
    };
    
    Storage& GetStorage()
    {
        BOOST_ASSERT(buf_[maxSmallString] == magic);
        Storage* p = reinterpret_cast<Storage*>(&buf_[0]);
        return *p;
    }
    
    const Storage& GetStorage() const
    {
        BOOST_ASSERT(buf_[maxSmallString] == magic);
        const Storage *p = reinterpret_cast<const Storage*>(&buf_[0]);
        return *p;
    }
    
    bool Small() const
    {
        return buf_[maxSmallString] != magic;
    }
        
public:
  SmallStringOpt(const SmallStringOpt& s)
    {
        if (s.Small())
        {
            flex_string_details::pod_copy(
                s.buf_, 
                s.buf_ + s.size(), 
                buf_);
        }
        else
        {
            new(buf_) Storage(s.GetStorage());
        }
        buf_[maxSmallString] = s.buf_[maxSmallString];
    }
    
    SmallStringOpt(const allocator_type&)
    {
        buf_[maxSmallString] = maxSmallString;
    }
    
    SmallStringOpt(const value_type* s, size_type len, const allocator_type& a)
    {
        if (len <= maxSmallString)
        {
            flex_string_details::pod_copy(s, s + len, buf_);
            buf_[maxSmallString] = value_type(maxSmallString - len);
        }
        else
        {
            new(buf_) Storage(s, len, a);
            buf_[maxSmallString] = magic;
        }
    }

    SmallStringOpt(size_type len, value_type c, const allocator_type& a)
    {
        if (len <= maxSmallString)
        {
            flex_string_details::pod_fill(buf_, buf_ + len, c);
            buf_[maxSmallString] = value_type(maxSmallString - len);
        }
        else
        {
            new(buf_) Storage(len, c, a);
            buf_[maxSmallString] = magic;
        }
    }
    
    SmallStringOpt& operator=(const SmallStringOpt& rhs)
    {
        reserve(rhs.size());
        resize(0, 0);
        append(rhs.data(), rhs.size());
        return *this;
    }

    ~SmallStringOpt()
    {
        if (!Small()) GetStorage().~Storage();
    }

    iterator begin()
    {
        if (Small()) return buf_;
        return &*GetStorage().begin(); 
    }
    
    const_iterator begin() const
    {
        if (Small()) return buf_;
        return &*GetStorage().begin(); 
    }
    
    iterator end()
    {
        if (Small()) return buf_ + maxSmallString - buf_[maxSmallString];
        return &*GetStorage().end(); 
    }
    
    const_iterator end() const
    {
        if (Small()) return buf_ + maxSmallString - buf_[maxSmallString];
        return &*GetStorage().end(); 
    }
    
    size_type size() const
    {
        BOOST_ASSERT(!Small() || maxSmallString >= buf_[maxSmallString]);
        return Small() 
            ? maxSmallString - buf_[maxSmallString] 
            : GetStorage().size();
    }

    size_type max_size() const
    { return get_allocator().max_size(); }

    size_type capacity() const
    { return Small() ? maxSmallString : GetStorage().capacity(); }

    void reserve(size_type res_arg)
    {
        if (Small())
        {
            if (res_arg <= maxSmallString) return;
            SmallStringOpt temp(*this);
            this->~SmallStringOpt();
            new(buf_) Storage(temp.data(), temp.size(), 
                temp.get_allocator());
            buf_[maxSmallString] = magic;
            GetStorage().reserve(res_arg);
        }
        else
        {
            GetStorage().reserve(res_arg);
        }
        BOOST_ASSERT(capacity() >= res_arg);
    }
    
    void append(const value_type* s, size_type sz)
    {
        if (!Small())
        {
            GetStorage().append(s, sz);
        }
        else
        {
            // append to a small string
            const size_type neededCapacity = 
                maxSmallString - buf_[maxSmallString] + sz;

            if (maxSmallString < neededCapacity)
            {
                // need to change storage strategy
                allocator_type alloc;
                Storage temp(alloc);
                temp.reserve(neededCapacity);
                temp.append(buf_, maxSmallString - buf_[maxSmallString]);
                temp.append(s, sz);
                buf_[maxSmallString] = magic;
                new(buf_) Storage(temp.get_allocator());
                GetStorage().swap(temp);
            }
            else
            {
                flex_string_details::pod_move(s, s + sz, 
                    buf_ + maxSmallString - buf_[maxSmallString]);
                buf_[maxSmallString] -= value_type(sz);
            }
        }
    }
    
    template <class InputIterator>
    void append(InputIterator b, InputIterator e)
    {
        // @@@ todo: optimize this depending on iterator type
        for (; b != e; ++b)
        {
            *this += *b;
        }
    }

    void resize(size_type n, value_type c)
    {
        if (Small())
        {
            if (n > maxSmallString)
            {
                // Small string resized to big string
                SmallStringOpt temp(*this); // can't throw
                // 11-17-2001: correct exception safety bug
                Storage newString(temp.data(), temp.size(), 
                    temp.get_allocator());
                newString.resize(n, c);
                // We make the reasonable assumption that an empty Storage
                //     constructor won't throw
                this->~SmallStringOpt();
                new(&buf_[0]) Storage(temp.get_allocator());
                buf_[maxSmallString] = value_type(magic);
                GetStorage().swap(newString);
            }
            else
            {
                // Small string resized to small string
                // 11-17-2001: bug fix: terminating zero not copied
                size_type toFill = n > size() ? n - size() : 0;
                flex_string_details::pod_fill(end(), end() + toFill, c);
                buf_[maxSmallString] = value_type(maxSmallString - n);
            }
        }
        else
        {
            if (n > maxSmallString)
            {
                // Big string resized to big string
                GetStorage().resize(n, c);
            }
            else
            {
                // Big string resized to small string
                // 11-17=2001: bug fix in the BOOST_ASSERTion below
                BOOST_ASSERT(capacity() > n);
                SmallStringOpt newObj(data(), n, get_allocator());
                newObj.swap(*this);
            }
        }
    }

    void swap(SmallStringOpt& rhs)
    {
        if (Small())
        {
            if (rhs.Small())
            {
                // Small swapped with small
                std::swap_ranges(buf_, buf_ + maxSmallString + 1, 
                    rhs.buf_);
            }
            else
            {
                // Small swapped with big
                // Make a copy of myself - can't throw
                SmallStringOpt temp(*this);
                // Nuke myself
                this->~SmallStringOpt();
                // Make an empty storage for myself (likely won't throw)
                new(buf_) Storage(0, value_type(), rhs.get_allocator());
                buf_[maxSmallString] = magic;
                // Recurse to this same function
                swap(rhs);
                // Nuke rhs
                rhs.~SmallStringOpt();
                // Build the new small string into rhs
                new(&rhs) SmallStringOpt(temp);
            }
        }
        else
        {
            if (rhs.Small())
            {
                // Big swapped with small
                // Already implemented, recurse with reversed args
                rhs.swap(*this);
            }
            else
            {
                // Big swapped with big
                GetStorage().swap(rhs.GetStorage());
            }
        }
    }
    
    const value_type* c_str() const
    { 
        if (!Small()) return GetStorage().c_str(); 
        buf_[maxSmallString - buf_[maxSmallString]] = value_type();
        return buf_;
    }

    const value_type* data() const
    { return Small() ? buf_ : GetStorage().data(); }
    
    allocator_type get_allocator() const
    { return allocator_type(); }
};

////////////////////////////////////////////////////////////////////////////////
// class template CowString
// Implements Copy on Write over any storage
////////////////////////////////////////////////////////////////////////////////

template <
    typename Storage, 
    typename Align = BOOST_DEDUCED_TYPENAME Storage::value_type*
>
class CowString
{
    typedef typename Storage::value_type E;
    typedef typename flex_string_details::get_unsigned<E>::result RefCountType;

public:
    typedef E value_type;
    typedef typename Storage::iterator iterator;
    typedef typename Storage::const_iterator const_iterator;
    typedef typename Storage::allocator_type allocator_type;
    typedef typename allocator_type::size_type size_type;
    
private:
    union
    {
        mutable char buf_[sizeof(Storage)];
        Align align_;
    };

    Storage& Data() const
    { return *reinterpret_cast<Storage*>(buf_); }

    RefCountType GetRefs() const
    {
        const Storage& d = Data();
        BOOST_ASSERT(d.size() > 0);
        BOOST_ASSERT(static_cast<RefCountType>(*d.begin()) != 0);
        return *d.begin();
    }
    
    RefCountType& Refs()
    {
        Storage& d = Data();
        BOOST_ASSERT(d.size() > 0);
        return reinterpret_cast<RefCountType&>(*d.begin());
    }
    
    void MakeUnique() const
    {
        BOOST_ASSERT(GetRefs() >= 1);
        if (GetRefs() == 1) return;

        union
        {
            char buf_[sizeof(Storage)];
            Align align_;
        } temp;

        new(buf_) Storage(
            *new(temp.buf_) Storage(Data()), 
            flex_string_details::Shallow());
        *Data().begin() = 1;
    }

public:
    CowString(const CowString& s)
    {
        if (s.GetRefs() == (std::numeric_limits<RefCountType>::max)())
        {
            // must make a brand new copy
            new(buf_) Storage(s.Data()); // non shallow
            Refs() = 1;
        }
        else
        {
            new(buf_) Storage(s.Data(), flex_string_details::Shallow());
            ++Refs();
        }
        BOOST_ASSERT(Data().size() > 0);
    }
    
    CowString(const allocator_type& a)
    {
        new(buf_) Storage(1, 1, a);
    }
    
    CowString(const E* s, size_type len, const allocator_type& a)
    {
        // Warning - MSVC's debugger has trouble tracing through the code below.
        // It seems to be a const-correctness issue
        //
        new(buf_) Storage(a);
        Data().reserve(len + 1);
        Data().resize(1, 1);
        Data().append(s, len);
    }

    CowString(size_type len, E c, const allocator_type& a)
    {
        new(buf_) Storage(len + 1, c, a);
        Refs() = 1;
    }
    
    CowString& operator=(const CowString& rhs)
    {
//        CowString(rhs).swap(*this);
        if (--Refs() == 0) Data().~Storage();
        if (rhs.GetRefs() == (std::numeric_limits<RefCountType>::max)())
        {
            // must make a brand new copy
            new(buf_) Storage(rhs.Data()); // non shallow
            Refs() = 1;
        }
        else
        {
            new(buf_) Storage(rhs.Data(), flex_string_details::Shallow());
            ++Refs();
        }
        BOOST_ASSERT(Data().size() > 0);
        return *this;
    }

    ~CowString()
    {
        BOOST_ASSERT(Data().size() > 0);
        if (--Refs() == 0) Data().~Storage();
    }

    iterator begin()
    {
        BOOST_ASSERT(Data().size() > 0);
        MakeUnique();
        return Data().begin() + 1; 
    }
    
    const_iterator begin() const
    {
        BOOST_ASSERT(Data().size() > 0);
        return Data().begin() + 1; 
    }
    
    iterator end()
    {
        MakeUnique();
        return Data().end(); 
    }
    
    const_iterator end() const
    {
        return Data().end(); 
    }
    
    size_type size() const
    {
        BOOST_ASSERT(Data().size() > 0);
        return Data().size() - 1;
    }

    size_type max_size() const
    { 
        BOOST_ASSERT(Data().max_size() > 0);
        return Data().max_size() - 1;
    }

    size_type capacity() const
    { 
        BOOST_ASSERT(Data().capacity() > 0);
        return Data().capacity() - 1;
    }

    void resize(size_type n, E c)
    {
        BOOST_ASSERT(Data().size() > 0);
        MakeUnique();
        Data().resize(n + 1, c);
    }

    void append(const E* s, size_type sz)
    {
        MakeUnique();
        Data().append(s, sz);
    }
    
    template <class InputIterator>
    void append(InputIterator b, InputIterator e)
    {
        MakeUnique();
        // @@@ todo: optimize this depending on iterator type
        for (; b != e; ++b)
        {
            *this += *b;
        }
    }

    void reserve(size_type res_arg)
    {
        if (capacity() > res_arg) return;
        MakeUnique();
        Data().reserve(res_arg + 1);
    }
    
    void swap(CowString& rhs)
    {
        Data().swap(rhs.Data());
    }
    
    const E* c_str() const
    { 
        BOOST_ASSERT(Data().size() > 0);
        return Data().c_str() + 1;
    }

    const E* data() const
    { 
        BOOST_ASSERT(Data().size() > 0);
        return Data().data() + 1;
    }
    
    allocator_type get_allocator() const
    { 
        return Data().get_allocator();
    }
};

////////////////////////////////////////////////////////////////////////////////
// class template flex_string
// a std::basic_string compatible implementation 
// Uses a Storage policy 
////////////////////////////////////////////////////////////////////////////////

template <typename E,
    class T = std::char_traits<E>,
    class A = std::allocator<E>,
    class Storage = AllocatorStringStorage<E, A> >
class flex_string : private Storage
{
#if defined(THROW_ON_ENFORCE)
    template <typename Exception>
    static void Enforce(bool condition, Exception*, const char* msg)
    { if (!condition) throw Exception(msg); }
#else
    template <typename Exception>
    static inline void Enforce(bool condition, Exception*, const char* msg)
    { BOOST_ASSERT(condition && msg); }
#endif // defined(THROW_ON_ENFORCE)

    bool Sane() const
    {
        return
            begin() <= end() &&
            empty() == (size() == 0) &&
            empty() == (begin() == end()) &&
            size() <= max_size() &&
            capacity() <= max_size() &&
            size() <= capacity();
    }

    struct Invariant;
    friend struct Invariant;
    struct Invariant
    {
        Invariant(const flex_string& s) : s_(s)
        {
            BOOST_ASSERT(s_.Sane());
        }
        ~Invariant()
        {
            BOOST_ASSERT(s_.Sane());
        }
    private:
        const flex_string& s_;
    };
    
public:
    // types
    typedef T traits_type;
    typedef typename traits_type::char_type value_type;
    typedef A allocator_type;
    typedef typename A::size_type size_type;
    typedef typename A::difference_type difference_type;
    
    typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename A::pointer pointer;
    typedef typename A::const_pointer const_pointer;
    
    typedef typename Storage::iterator iterator;
    typedef typename Storage::const_iterator const_iterator;

    typedef boost::reverse_iterator<iterator> reverse_iterator;
    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos;    // = size_type(-1)

private:
    static size_type Min(size_type lhs, size_type rhs)
    { return lhs < rhs ? lhs : rhs; }
    
public:    
    // 21.3.1 construct/copy/destroy
    explicit flex_string(const A& a = A())
    : Storage(a) 
    {}
    
    flex_string(const flex_string& str)
    : Storage(str) 
    {
    }
    
    flex_string(const flex_string& str, size_type pos, 
        size_type n = npos, const A& a = A())
    : Storage(a) 
    {
        Enforce(pos <= str.size(), (std::out_of_range*)0, "");
        assign(str, pos, n);
    }
    
    flex_string(const value_type* s, const A& a = A())
    : Storage(s, traits_type::length(s), a)
    {}
    
    flex_string(const value_type* s, size_type n, const A& a = A())
    : Storage(s, n, a)
    {}
    
    flex_string(size_type n, value_type c, const A& a = A())
    : Storage(n, c, a)
    {}

    template <class InputIterator>
    flex_string(InputIterator begin, InputIterator end, const A& a = A())
    : Storage(a)
    {
        assign(begin, end);
    }

    ~flex_string()
    {}
    
    flex_string& operator=(const flex_string& str)
    {
        if (this != &str) {
            Storage& s = *this;
            s = str;
        }
        return *this;
    }   
    
    flex_string& operator=(const value_type* s)
    {
        assign(s);
        return *this;
    }

    flex_string& operator=(value_type c)
    {
        assign(1, c);
        return *this;
    }
    
    // 21.3.2 iterators:
    iterator begin()
    { return Storage::begin(); }
    
    const_iterator begin() const
    { return Storage::begin(); }
    
    iterator end()
    { return Storage::end(); }
    
    const_iterator end() const
    { return Storage::end(); }

    reverse_iterator rbegin()
    { return reverse_iterator(end()); }
    
    const_reverse_iterator rbegin() const
    { return const_reverse_iterator(end()); }
    
    reverse_iterator rend()
    { return reverse_iterator(begin()); }
    
    const_reverse_iterator rend() const
    { return const_reverse_iterator(begin()); }
    
    // 21.3.3 capacity:
    size_type size() const
    { return Storage::size(); }
    
    size_type length() const
    { return size(); }
    
    size_type max_size() const
    { return Storage::max_size(); }

    void resize(size_type n, value_type c)
    { Storage::resize(n, c); }
    
    void resize(size_type n)
    { resize(n, value_type()); }
    
    size_type capacity() const
    { return Storage::capacity(); }
    
    void reserve(size_type res_arg = 0)
    {
        Enforce(res_arg <= max_size(), (std::length_error*)0, "");
        Storage::reserve(res_arg);
    }
    
    void clear()
    { resize(0); } 
    
    bool empty() const
    { return size() == 0; }
    
    // 21.3.4 element access:
    const_reference operator[](size_type pos) const
    { return *(begin() + pos); }
    
    reference operator[](size_type pos)
    { return *(begin() + pos); }

    const_reference at(size_type n) const
    {
        Enforce(n < size(), (std::out_of_range*)0, "");
        return (*this)[n];
    }
    
    reference at(size_type n)
    {
        Enforce(n < size(), (std::out_of_range*)0, "");
        return (*this)[n];
    }
    
    // 21.3.5 modifiers:
    flex_string& operator+=(const flex_string& str)
    { return append(str); }
    
    flex_string& operator+=(const value_type* s)
    { return append(s); }

    flex_string& operator+=(value_type c)
    { 
        const size_type cap = capacity();
        if (size() == cap)
        {
            reserve(cap << 1u);
        }
        resize(size() + 1, c);
        return *this;
    }
    
    flex_string& append(const flex_string& str)
    { return append(str, 0, npos); }
    
    flex_string& append(const flex_string& str, size_type pos,
        size_type n)
    { 
        const size_type sz = str.size();
        Enforce(pos <= sz, (std::out_of_range*)0, "");
        return append(str.c_str() + pos, Min(n, sz - pos)); 
    }
    
    flex_string& append(const value_type* s, size_type n)
    { 
        Storage::append(s, n); 
        return *this;
    }
    
    flex_string& append(const value_type* s)
    { return append(s, traits_type::length(s)); }
    
    flex_string& append(size_type n, value_type c)
    { 
        resize(size() + n, c);
        return *this;
    }
/*    
    template<class InputIterator>
    flex_string& append(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first) *this += E(*first);
        return *this;
    }
*/    
    void push_back(value_type c)
    { 
        *this += c;
    }

    flex_string& assign(const flex_string& str)
    { 
        if (&str == this) return *this;
        replace(0, size(), &*str.begin(), str.size());
        return *this;
    }
    
    flex_string& assign(const flex_string& str, size_type pos,
        size_type n)
    { 
        Enforce(pos <= str.size(), (std::out_of_range*)0, "");
        return assign(str.data() + pos, Min(n, str.size() - pos));
    }
    
    flex_string& assign(const value_type* s, size_type n)
    {
        if (size() >= n)
        {
            flex_string_details::pod_move(s, s + n, &*begin());
            resize(n, value_type());
        }
        else
        {
            flex_string_details::pod_move(s, s + size(), &*begin());
            Storage::append(s + size(), n - size());
        }
        return *this;
    }
    
    flex_string& assign(const value_type* s)
    { return assign(s, traits_type::length(s)); }
    
    flex_string& assign(size_type n, value_type c)
    { return replace(begin(), end(), n, c); } 
    
    template<class InputIterator>
    flex_string& assign(InputIterator first, InputIterator last)
    { return replace(begin(), end(), first, last); }
    
    flex_string& insert(size_type pos1, const flex_string& str)
    { return insert(pos1, str, 0, npos); }
    
    flex_string& insert(size_type pos1, const flex_string& str,
        size_type pos2, size_type n)
    { return replace(pos1, 0, str, pos2, n); }
    
    flex_string& insert(size_type pos, const value_type* s, size_type n)
    { return replace(pos, 0, s, n); }
    
    flex_string& insert(size_type pos, const value_type* s)
    { return insert(pos, s, traits_type::length(s)); }
    
    flex_string& insert(size_type pos, size_type n, value_type c)
    { return replace(pos, 0, n, c); }
    
    iterator insert(iterator p, value_type c = value_type()) 
    {
        const size_type pos = p - begin();
        insert(pos, &c, 1);
        return begin() + pos;
    }
    
    void insert(iterator p, size_type n, value_type c)
    { insert(p - begin(), n, c); }
    
    template<class InputIterator>
    void insert(iterator p, InputIterator first, InputIterator last)
    { replace(p, p, first, last); }
    
    flex_string& erase(size_type pos = 0, size_type n = npos)
    { 
        return replace(pos, Min(n, size() - pos), 0, value_type()); 
    }
    
    iterator erase(iterator position)
    {
        const size_type pos(position - begin());
        erase(pos, 1);
        return begin() + pos;
    }
    
    iterator erase(iterator first, iterator last)
    {
        const size_type pos(first - begin());
        erase(pos, last - first);
        return begin() + pos;
    }

    // @@@ replace

    flex_string& replace(size_type pos1, size_type n1, const flex_string& str)
    { return replace(pos1, n1, str, 0, npos); }
    
    flex_string& replace(size_type pos1, size_type n1, const flex_string& str,
        size_type pos2, size_type n2)
    {
        Enforce(pos1 <= length() && pos2 <= str.length(), 
            (std::out_of_range*)0, "");
        return replace(pos1, n1, &*str.begin() + pos2, 
            Min(n2, str.length() - pos2));
    }
    
    flex_string& replace(const size_type d, size_type n1, const value_type* s1,
        const size_type n2)
    {
        using namespace flex_string_details;
        Enforce(d <= size(), (std::out_of_range*)0, "");
        if (d + n1 > size()) n1 = size() - d;
        const int delta = int(n2 - n1);
        static const std::less_equal<const value_type*> le = 
            std::less_equal<const value_type*>();
        const bool aliased = le(&*begin(), s1) && le(s1, &*end());

        if (delta > 0)
        {
            if (capacity() < size() + delta)
            {
                // realloc the string
                if (aliased)
                {
                    const size_type offset = s1 - &*begin();
                    reserve(size() + delta);
                    s1 = &*begin() + offset;
                }
                else
                {
                    reserve(size() + delta);
                }
            }

            const value_type* s2 = s1 + n2;
            value_type* d1 = &*begin() + d;
            value_type* d2 = d1 + n1;

            const int tailLen = int(&*end() - d2);

            if (delta <= tailLen)
            {
                value_type* oldEnd = &*end();
                // simple case
                Storage::append(oldEnd - delta, delta);

                pod_move(d2, d2 + (tailLen - delta), d2 + delta);
                if (le(d2, s1))
                {
                    if (aliased)
                    {
                        pod_copy(s1 + delta, s2 + delta, d1);
                    }
                    else
                    {
                        pod_copy(s1, s2, d1);
                    }
                }
                else
                {
                    // d2 > s1
                    if (le(d2, s2))
                    {
                        BOOST_ASSERT(aliased);
                        pod_move(s1, d2, d1);
                        pod_move(d2 + delta, s2 + delta, d1 + (d2 - s1));
                    }
                    else
                    {
                        pod_move(s1, s2, d1);
                    }
                }
            }
            else
            {
                const size_type sz = delta - tailLen;
                Storage::append(s2 - sz, sz);
                Storage::append(d2, tailLen);
                pod_move(s1, s2 - (delta - tailLen), d1);
            }
        }
        else
        {
            pod_move(s1, s1 + n2, &*begin() + d);
            pod_move(&*begin() + d + n1, &*end(), &*begin() + d + n1 + delta);
            resize(size() + delta);
        }
        return *this;
    }

    flex_string& replace(size_type pos, size_type n1, const value_type* s)
    { return replace(pos, n1, s, traits_type::length(s)); }
    
    flex_string& replace(size_type pos, size_type n1, size_type n2, 
        value_type c)
    {
        if (pos + n1 > size()) n1 = size() - pos;
        const size_type oldSize = size();
        if (pos + n2 > oldSize)
        {
            resize(pos + n2, c);
            Storage::append(&*begin() + pos + n1, oldSize - pos - n1);
            flex_string_details::pod_fill(&*begin() + pos, 
                &*begin() + oldSize, c);
        }
        else
        {
            if (n2 > n1)
            {
                const size_type delta = n2 - n1;
                Storage::append(&*begin() + oldSize - delta, delta);
                flex_string_details::pod_move(
                    &*begin() + pos + n1, 
                    &*begin() + oldSize - delta, 
                    &*begin() + pos + n2);
            }
            else
            {
                flex_string_details::pod_move(&*begin() + pos + n1, &*end(), 
                    &*begin() + pos + n2);
                resize(oldSize - (n1 - n2));
            }
            flex_string_details::pod_fill(&*begin() + pos, 
                &*begin() + pos + n2, c);
        }
        return *this;
    }
        
    flex_string& replace(iterator i1, iterator i2, const flex_string& str)
    { return replace(i1, i2, str.c_str(), str.length()); }
    
    flex_string& replace(iterator i1, iterator i2, 
        const value_type* s, size_type n)
    { return replace(i1 - begin(), i2 - i1, s, n); }
    
    flex_string& replace(iterator i1, iterator i2, const value_type* s)
    { return replace(i1, i2, s, traits_type::length(s)); }
    
    flex_string& replace(iterator i1, iterator i2,
        size_type n, value_type c)
    { return replace(i1 - begin(), i2 - i1, n, c); }
    
private:
    template <int i> class Selector {};
    
//    template <class U1, class U2> struct SameType 
//    {
//        enum { result = false };
//    };
//    
//    template <class U> struct SameType<U, U>
//    {
//        enum { result = true };
//    };
    
    template<class ReallyAnIntegral>
    flex_string& ReplaceImpl(iterator i1, iterator i2,
        ReallyAnIntegral n, ReallyAnIntegral c, Selector<1>)
    { 
        return replace(i1, i2, static_cast<size_type>(n), 
            static_cast<value_type>(c)); 
    }    

    template<class InputIterator>
    flex_string& ReplaceImpl(iterator i1, iterator i2,
        InputIterator b, InputIterator e, Selector<0>)
    { 
        BOOST_ASSERT(false);
        return *this;
    }    

public:
    template<class InputIterator>
    flex_string& replace(iterator i1, iterator i2,
        InputIterator j1, InputIterator j2)
    { 
        return ReplaceImpl(i1, i2, j1, j2, 
            Selector<std::numeric_limits<InputIterator>::is_specialized>()); 
    }
       
    size_type copy(value_type* s, size_type n, size_type pos = 0) const
    {
        Enforce(pos <= size(), (std::out_of_range*)0, "");
        n = Min(n, size() - pos);
        
        flex_string_details::pod_copy(
            &*begin() + pos,
            &*begin() + pos + n,
            s);
        return n;
    }
    
    void swap(flex_string& rhs)
    {
        Storage& srhs = rhs;
        this->Storage::swap(srhs);
    }
    
    // 21.3.6 string operations:
    const value_type* c_str() const
    { return Storage::c_str(); }
    
    const value_type* data() const
    { return Storage::data(); }
    
    allocator_type get_allocator() const
    { return Storage::get_allocator(); }
    
    size_type find(const flex_string& str, size_type pos = 0) const
    { return find(str.data(), pos, str.length()); }
    
    size_type find (const value_type* s, size_type pos, size_type n) const
    {
        for (; pos <= size(); ++pos)
        {
            if (traits_type::compare(&*begin() + pos, s, n) == 0)
            {
                return pos;
            }
        }
        return npos;
    }
    
    size_type find (const value_type* s, size_type pos = 0) const
    { return find(s, pos, traits_type::length(s)); }

    size_type find (value_type c, size_type pos = 0) const
    { return find(&c, pos, 1); }
    
    size_type rfind(const flex_string& str, size_type pos = npos) const
    { return rfind(str.c_str(), pos, str.length()); }
    
    size_type rfind(const value_type* s, size_type pos, size_type n) const
    {
        if (n > length()) return npos;
        pos = Min(pos, length() - n);
        if (n == 0) return pos;

        const_iterator i(begin() + pos);
        for (; ; --i)
        {
            if (traits_type::eq(*i, *s) 
                && traits_type::compare(&*i, s, n) == 0)
            {
                return i - begin();
            }
            if (i == begin()) break;
        }
        return npos;
    }

    size_type rfind(const value_type* s, size_type pos = npos) const
    { return rfind(s, pos, traits_type::length(s)); }

    size_type rfind(value_type c, size_type pos = npos) const
    { return rfind(&c, pos, 1); }
    
    size_type find_first_of(const flex_string& str, size_type pos = 0) const
    { return find_first_of(str.c_str(), pos, str.length()); }
    
    size_type find_first_of(const value_type* s, 
        size_type pos, size_type n) const
    {
        if (pos > length() || n == 0) return npos;
        const_iterator i(begin() + pos),
            finish(end());
        for (; i != finish; ++i)
        {
            if (traits_type::find(s, n, *i) != 0)
            {
                return i - begin();
            }
        }
        return npos;
    }
        
    size_type find_first_of(const value_type* s, size_type pos = 0) const
    { return find_first_of(s, pos, traits_type::length(s)); }
    
    size_type find_first_of(value_type c, size_type pos = 0) const
    { return find_first_of(&c, pos, 1); }
    
    size_type find_last_of (const flex_string& str,
        size_type pos = npos) const
    { return find_last_of(str.c_str(), pos, str.length()); }
    
    size_type find_last_of (const value_type* s, size_type pos, 
        size_type n) const
    {
        if (!empty() && n > 0)
        {
            pos = Min(pos, length() - 1);
            const_iterator i(begin() + pos);
            for (;; --i)
            {
                if (traits_type::find(s, n, *i) != 0)
                {
                    return i - begin();
                }
                if (i == begin()) break;
            }
        }
        return npos;
    }

    size_type find_last_of (const value_type* s, 
        size_type pos = npos) const
    { return find_last_of(s, pos, traits_type::length(s)); }

    size_type find_last_of (value_type c, size_type pos = npos) const
    { return find_last_of(&c, pos, 1); }
    
    size_type find_first_not_of(const flex_string& str,
        size_type pos = 0) const
    { return find_first_not_of(str.data(), pos, str.size()); }
    
    size_type find_first_not_of(const value_type* s, size_type pos,
        size_type n) const
    {
        if (pos < length())
        {
            const_iterator 
                i(begin() + pos),
                finish(end());
            for (; i != finish; ++i)
            {
                if (traits_type::find(s, n, *i) == 0)
                {
                    return i - begin();
                }
            }
        }
        return npos;
    }
    
    size_type find_first_not_of(const value_type* s, 
        size_type pos = 0) const
    { return find_first_not_of(s, pos, traits_type::length(s)); }
        
    size_type find_first_not_of(value_type c, size_type pos = 0) const
    { return find_first_not_of(&c, pos, 1); }
    
    size_type find_last_not_of(const flex_string& str,
        size_type pos = npos) const
    { return find_last_not_of(str.c_str(), pos, str.length()); }
    
    size_type find_last_not_of(const value_type* s, size_type pos,
        size_type n) const
    {
        if (!empty())
        {
            pos = Min(pos, size() - 1);
            const_iterator i(begin() + pos);
            for (;; --i)
            {
                if (traits_type::find(s, n, *i) == 0)
                {
                    return i - begin();
                }
                if (i == begin()) break;
            }
        }
        return npos;
    }

    size_type find_last_not_of(const value_type* s, 
        size_type pos = npos) const
    { return find_last_not_of(s, pos, traits_type::length(s)); }
    
    size_type find_last_not_of (value_type c, size_type pos = npos) const
    { return find_last_not_of(&c, pos, 1); }
    
    flex_string substr(size_type pos = 0, size_type n = npos) const
    {
        Enforce(pos <= size(), (std::out_of_range*)0, "");
        return flex_string(data() + pos, Min(n, size() - pos));
    }

    std::ptrdiff_t compare(const flex_string& str) const
    { return compare(0, size(), str.data(), str.length()); }
    
    std::ptrdiff_t compare(size_type pos1, size_type n1,
        const flex_string& str) const
    { return compare(pos1, n1, str.data(), str.size()); }
    
    std::ptrdiff_t compare(size_type pos1, size_type n1,
        const value_type* s, size_type n2 = npos) const
    {
        Enforce(pos1 <= size(), (std::out_of_range*)0, "");

        n1 = Min(size() - pos1, n1);
        const std::ptrdiff_t result = traits_type::compare(data() + pos1, s, Min(n1, n2));
        return (result != 0) ? result : int(n1 - n2);
    }
    
    std::ptrdiff_t compare(size_type pos1, size_type n1,
        const flex_string& str,
        size_type pos2, size_type n2) const
    {
        Enforce(pos2 <= str.size(), (std::out_of_range*)0, "");
        return compare(pos1, n1, str.data() + pos2, Min(n2, str.size() - pos2));
    }

    std::ptrdiff_t compare(const value_type* s) const
    { return compare(0, size(), s, traits_type::length(s)); }
};

// non-member functions
template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs);
    result.append(rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    const typename flex_string<E, T, A, S>::size_type len = 
        flex_string<E, T, A, S>::traits_type::length(lhs);
    result.reserve(len + rhs.size());
    result.append(lhs, len);
    result.append(rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(
    typename flex_string<E, T, A, S>::value_type lhs, 
    const flex_string<E, T, A, S>& rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(1 + rhs.size());
    result.push_back(lhs);
    result.append(rhs);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{
    typedef typename flex_string<E, T, A, S>::size_type size_type;
    typedef typename flex_string<E, T, A, S>::traits_type traits_type;

    flex_string<E, T, A, S> result;
    const size_type len = traits_type::length(rhs);
    result.reserve(lhs.size() + len);
    result.append(lhs);
    result.append(rhs, len);
    return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    typename flex_string<E, T, A, S>::value_type rhs)
{
    flex_string<E, T, A, S> result;
    result.reserve(lhs.size() + 1);
    result.append(lhs);
    result.push_back(rhs);
    return result;
}

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return lhs.compare(rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator==(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs == lhs; }

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return lhs.compare(rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return lhs.compare(rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return lhs.compare(rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs.compare(lhs) > 0; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, 
    const typename flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const typename flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

// subclause 21.3.7.8:
//void swap(flex_string<E, T, A, S>& lhs, flex_string<E, T, A, S>& rhs);    // to do

template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>&
operator>>(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str);

template <typename E, class T, class A, class S>
std::basic_ostream<typename flex_string<E, T, A, S>::value_type,
    typename flex_string<E, T, A, S>::traits_type>&
operator<<(
    std::basic_ostream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>& os,
    const flex_string<E, T, A, S>& str)
{ return os << str.c_str(); }

template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type,
    typename flex_string<E, T, A, S>::traits_type>&
getline(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
        typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str,
    typename flex_string<E, T, A, S>::value_type delim);

template <typename E, class T, class A, class S>
std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
    typename flex_string<E, T, A, S>::traits_type>&
getline(
    std::basic_istream<typename flex_string<E, T, A, S>::value_type, 
        typename flex_string<E, T, A, S>::traits_type>& is,
    flex_string<E, T, A, S>& str);

template <typename E1, class T, class A, class S>
const typename flex_string<E1, T, A, S>::size_type
flex_string<E1, T, A, S>::npos = (typename flex_string<E1, T, A, S>::size_type)(-1);

///////////////////////////////////////////////////////////////////////////////
}   // namespace util
}   // namespace wave
}   // namespace boost

#endif // FLEX_STRING_INC_
