#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

struct EdgeData {
    int id, source, target;
    double cost, reverse_cost;
};

struct IsoResult {
    bool found;
    std::vector<std::map<int, int> > mappings;
    IsoResult() : found(false) {}
};

struct CollectCallback {
    const Graph& query;
    const Graph& target;
    const std::map<Vertex, int>& q_id;
    const std::map<Vertex, int>& t_id;
    IsoResult& result;

    template<typename Map1, typename Map2>
    bool operator()(Map1 f, Map2) const {
        std::map<int, int> m;
        boost::graph_traits<Graph>::vertex_iterator vi, ve;
        for (boost::tie(vi, ve) = boost::vertices(query); vi != ve; ++vi)
            m[q_id.at(*vi)] = t_id.at(get(f, *vi));
        result.mappings.push_back(m);
        result.found = true;
        return true;
    }
};

Graph build_graph(const std::vector<EdgeData>& edges,
                  std::map<int, Vertex>& vmap,
                  std::map<Vertex, int>& id_map) {
    Graph g;
    std::set<int> nodes;
    for (size_t i = 0; i < edges.size(); ++i) {
        nodes.insert(edges[i].source);
        nodes.insert(edges[i].target);
    }
    for (std::set<int>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        Vertex v = add_vertex(g);
        vmap[*it] = v;
        id_map[v] = *it;
    }
    for (size_t i = 0; i < edges.size(); ++i)
        if (edges[i].cost > 0)
            add_edge(vmap[edges[i].source], vmap[edges[i].target], g);
    return g;
}

IsoResult apply_vftwo_subgraph_isomorphism(const std::vector<EdgeData>& target_edges,
                                            const std::vector<EdgeData>& query_edges) {
    std::map<int, Vertex> tmap, qmap;
    std::map<Vertex, int> t_id, q_id;

    Graph target = build_graph(target_edges, tmap, t_id);
    Graph query  = build_graph(query_edges,  qmap, q_id);

    IsoResult result;
    if (num_vertices(query) > num_vertices(target))
        return result;

    CollectCallback cb = {query, target, q_id, t_id, result};
    boost::vf2_subgraph_iso(
        query, target, cb,
        get(boost::vertex_index, query),
        get(boost::vertex_index, target),
        boost::vertex_order_by_mult(query),
        boost::always_equivalent(),
        boost::always_equivalent()
    );
    return result;
}

int main() {
    std::vector<EdgeData> G;
    G.push_back((EdgeData){1,  1,  3,  6,  6}); G.push_back((EdgeData){2,  3,  7,  3,  3});
    G.push_back((EdgeData){3,  2,  4, 17, 17}); G.push_back((EdgeData){4,  4,  9, 14, 14});
    G.push_back((EdgeData){5,  8,  4, 14, 14}); G.push_back((EdgeData){6,  7,  8, 10, 10});
    G.push_back((EdgeData){7,  6,  7,  4,  4}); G.push_back((EdgeData){8,  5,  6,  1,  1});
    G.push_back((EdgeData){9,  7, 11,  8,  8}); G.push_back((EdgeData){10,11, 12, 11, 11});
    G.push_back((EdgeData){11, 8, 12, 12, 12}); G.push_back((EdgeData){12,13, 14, 18, 18});
    G.push_back((EdgeData){13,12, 17, 13, 13}); G.push_back((EdgeData){14,11, 16,  9,  9});
    G.push_back((EdgeData){15,16, 17, 15, 15}); G.push_back((EdgeData){16,10, 11,  5,  5});
    G.push_back((EdgeData){17,10, 15,  3,  3}); G.push_back((EdgeData){18,15, 16, 16, 16});
    G.push_back((EdgeData){19, 6, 10,  2,  2});

    std::vector<EdgeData> H;
    H.push_back((EdgeData){1, 2,  3, 1, 1});
    H.push_back((EdgeData){2, 3,  4, 1, 1});
    H.push_back((EdgeData){3, 4, 12, 1, 1});
    H.push_back((EdgeData){4,12,  2, 1, 1});

    IsoResult res = apply_vftwo_subgraph_isomorphism(G, H);

    // Deduplicate: rotations/reflections of the same cycle share the same node set
    std::set<std::set<int> > unique_cycles;
    for (size_t i = 0; i < res.mappings.size(); ++i) {
        std::set<int> nodes;
        for (std::map<int,int>::iterator it = res.mappings[i].begin(); it != res.mappings[i].end(); ++it)
            nodes.insert(it->second);
        unique_cycles.insert(nodes);
    }

    std::cout << "VF2 Subgraph Isomorphism: H in G" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "Result  : " << (res.found ? "MATCH FOUND" : "NO MATCH") << std::endl;
    std::cout << "Total mappings (incl. rotations/reflections): " << res.mappings.size() << std::endl;
    std::cout << "Unique 4-cycles in G: " << unique_cycles.size() << std::endl;
    std::cout << "---------------------------------" << std::endl;

    int idx = 1;
    for (std::set<std::set<int> >::iterator ci = unique_cycles.begin(); ci != unique_cycles.end(); ++ci, ++idx) {
        std::cout << "Cycle #" << idx << ": { ";
        for (std::set<int>::iterator ni = ci->begin(); ni != ci->end(); ++ni)
            std::cout << *ni << " ";
        std::cout << "}" << std::endl;
    }

    return 0;
}