//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_SMALLEST_LAST_VERTEX_ORDERING_HPP
#define BOOST_SMALLEST_LAST_VERTEX_ORDERING_HPP
/*
   The smallest-last ordering is defined for the loopless graph G with
   vertices a(j), j = 1,2,...,n where a(j) is the j-th column of A and
   with edge (a(i),a(j)) if and only if columns i and j have a
   non-zero in the same row position.  The smallest-last ordering is
   determined recursively by letting list(k), k = n,...,1 be a column
   with least degree in the subgraph spanned by the un-ordered
   columns.
 */
#include <vector>
#include <algorithm>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/pending/bucket_sorter.hpp>

namespace boost {

  template <class VertexListGraph, class Order, class Degree, class Marker>
  void 
  smallest_last_vertex_ordering(const VertexListGraph& G, Order order, 
                                Degree degree, Marker marker) {
    typedef typename graph_traits<VertexListGraph> GraphTraits;
    typedef typename GraphTraits::vertex_descriptor Vertex;
    //typedef typename GraphTraits::size_type size_type;
    typedef std::size_t size_type;
    
    const size_type num = num_vertices(G);
    
    typedef typename vertex_property_map<VertexListGraph, vertex_index_t>::type ID;
    typedef bucket_sorter<size_type, Vertex, Degree, ID> BucketSorter;
    
    BucketSorter degree_bucket_sorter(num, num, degree,  
                                      get_vertex_property(G, vertex_index));

    smallest_last_vertex_ordering(G, order, degree, marker, degree_bucket_sorter);
  }

  template <class VertexListGraph, class Order, class Degree, 
            class Marker, class BucketSorter>
  void 
  smallest_last_vertex_ordering(const VertexListGraph& G, Order order, 
                                Degree degree, Marker marker,
                                BucketSorter& degree_buckets) {
    typedef typename graph_traits<VertexListGraph> GraphTraits;
    typedef typename GraphTraits::vertex_descriptor Vertex;
    //typedef typename GraphTraits::size_type size_type;
    typedef std::size_t size_type;

    const size_type num = num_vertices(G);
    
    typename GraphTraits::vertex_iterator v, vend;
    for (boost::tie(v, vend) = vertices(G); v != vend; ++v) {
      put(marker, *v, num);
      put(degree, *v, out_degree(*v, G));
      degree_buckets.push(*v);
    }
 
    size_type minimum_degree = 1;
    size_type current_order = num - 1;
    
    while ( 1 ) {
      typedef typename BucketSorter::stack MDStack;
      MDStack minimum_degree_stack = degree_buckets[minimum_degree];
      while (minimum_degree_stack.empty())
        minimum_degree_stack = degree_buckets[++minimum_degree];
      
      Vertex node = minimum_degree_stack.top();
      put(order, current_order, node);
      
      if ( current_order == 0 ) //find all vertices
        break;
      
      minimum_degree_stack.pop();
      put(marker, node, 0); //node has been ordered.
      
      typename GraphTraits::adjacency_iterator v, vend;
      for (boost::tie(v,vend) = adjacent_vertices(node, G); v != vend; ++v)
        
        if ( get(marker,*v) > current_order ) { //*v is unordered vertex
          put(marker, *v, current_order);  //mark the columns adjacent to node
          
          //It is possible minimum degree goes down
          //Here we keep tracking it.
          put(degree, *v, get(degree, *v) - 1); 
          BOOST_USING_STD_MIN();
          minimum_degree = min BOOST_PREVENT_MACRO_SUBSTITUTION(minimum_degree, get(degree, *v)); 
          
          //update the position of *v in the bucket sorter
          degree_buckets.update(*v);
        }

      current_order--;
    }
    
    //at this point, order[i] = v_i;
  }
  
}

#endif

