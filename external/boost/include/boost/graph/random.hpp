//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Copyright (C) Vladimir Prus 2003
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_RANDOM_HPP
#define BOOST_GRAPH_RANDOM_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <boost/pending/property.hpp>
#include <boost/graph/properties.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <iostream>

namespace boost {

  // grab a random vertex from the graph's vertex set
  template <class Graph, class RandomNumGen>
  typename graph_traits<Graph>::vertex_descriptor
  random_vertex(Graph& g, RandomNumGen& gen)
  {
    if (num_vertices(g) > 1) {
      uniform_int<> distrib(0, num_vertices(g)-1);
      variate_generator<RandomNumGen&, uniform_int<> > rand_gen(gen, distrib);
      std::size_t n = rand_gen();
      typename graph_traits<Graph>::vertex_iterator
        i = vertices(g).first;
      while (n-- > 0) ++i; // std::advance not VC++ portable
      return *i;
    } else
      return *vertices(g).first;
  }

  template <class Graph, class RandomNumGen>
  typename graph_traits<Graph>::edge_descriptor
  random_edge(Graph& g, RandomNumGen& gen) {
    if (num_edges(g) > 1) {
      uniform_int<> distrib(0, num_edges(g)-1);
      variate_generator<RandomNumGen&, uniform_int<> > rand_gen(gen, distrib);
      typename graph_traits<Graph>::edges_size_type 
        n = rand_gen();
      typename graph_traits<Graph>::edge_iterator
        i = edges(g).first;
      while (n-- > 0) ++i; // std::advance not VC++ portable
      return *i;
    } else
      return *edges(g).first;
  }

  namespace detail {
    class dummy_property_copier {
    public:
      template<class V1, class V2>
      void operator()(const V1&, const V2&) const {}
    };
  }

  template <typename MutableGraph, class RandNumGen>
  void generate_random_graph1
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     bool allow_parallel = true,
     bool self_edges = false)
  {
    typedef graph_traits<MutableGraph> Traits;
    typedef typename Traits::vertices_size_type v_size_t;
    typedef typename Traits::edges_size_type e_size_t;
    typedef typename Traits::vertex_descriptor vertex_descriptor;

    // When parallel edges are not allowed, we create a new graph which
    // does not allow parallel edges, construct it and copy back.
    // This is not efficient if 'g' already disallow parallel edges,
    // but that's task for later.
    if (!allow_parallel) {

      typedef typename boost::graph_traits<MutableGraph>::directed_category dir;      
      typedef typename mpl::if_<is_convertible<dir, directed_tag>,
          directedS, undirectedS>::type select;
      adjacency_list<setS, vecS, select> g2;
      generate_random_graph1(g2, V, E, gen, true, self_edges);

      copy_graph(g2, g, vertex_copy(detail::dummy_property_copier()).
                        edge_copy(detail::dummy_property_copier()));

    } else {

      for (v_size_t i = 0; i < V; ++i)
        add_vertex(g);
      
      for (e_size_t j = 0; j < E; ++j) {
        vertex_descriptor a = random_vertex(g, gen), b;
        do {
          b = random_vertex(g, gen);
        } while (self_edges == false && a == b);
        add_edge(a, b, g);
      }
    }
  }

  template <typename MutableGraph, class RandNumGen>
  void generate_random_graph
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     bool allow_parallel = true,
     bool self_edges = false)
  {
      generate_random_graph1(g, V, E, gen, allow_parallel, self_edges);
  }

  template <typename MutableGraph, typename RandNumGen,
            typename VertexOutputIterator, typename EdgeOutputIterator>
  void generate_random_graph
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     VertexOutputIterator vertex_out,
     EdgeOutputIterator edge_out,
     bool self_edges = false)
  {
    typedef graph_traits<MutableGraph> Traits;
    typedef typename Traits::vertices_size_type v_size_t;
    typedef typename Traits::edges_size_type e_size_t;
    typedef typename Traits::vertex_descriptor vertex_t;
    typedef typename Traits::edge_descriptor edge_t;

    for (v_size_t i = 0; i < V; ++i)
      *vertex_out++ = add_vertex(g);

    for (e_size_t j = 0; j < E; ++j) {
      vertex_t a = random_vertex(g, gen), b;
      do {
        b = random_vertex(g, gen);
      } while (self_edges == false && a == b);
      edge_t e; bool inserted;
      tie(e, inserted) = add_edge(a, b, g);
      if (inserted)
        *edge_out++ = std::make_pair(source(e, g), target(e, g));
    }
  }

  namespace detail {

    template<class Property, class G, class RandomGenerator>
    void randomize_property(G& g, RandomGenerator& rg, 
                            Property, vertex_property_tag)
    {
      typename property_map<G, Property>::type pm = get(Property(), g);
      typename graph_traits<G>::vertex_iterator vi, ve;
      for (tie(vi, ve) = vertices(g); vi != ve; ++vi) {
        pm[*vi] = rg();
      }
    }

    template<class Property, class G, class RandomGenerator>
    void randomize_property(G& g, RandomGenerator& rg, 
                            Property, edge_property_tag)
    {
      typename property_map<G, Property>::type pm = get(Property(), g);
      typename graph_traits<G>::edge_iterator ei, ee;
      for (tie(ei, ee) = edges(g); ei != ee; ++ei) {
        pm[*ei] = rg();
      }
    }
  }

  template<class Property, class G, class RandomGenerator>
  void randomize_property(G& g, RandomGenerator& rg)
  {
    detail::randomize_property
        (g, rg, Property(), typename property_kind<Property>::type());
  }



  
}


#endif
