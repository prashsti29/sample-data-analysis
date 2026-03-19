#include <boost/config.hpp>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

using namespace boost;
using namespace std;

// Callback to print each isomorphism mapping
template <typename Graph1, typename Graph2>
struct my_callback {
    my_callback(const Graph1& graph1, const Graph2& graph2) 
        : graph1(graph1), graph2(graph2) {}

    template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
    bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
        cout << "Found isomorphism:" << endl;
        BGL_FORALL_VERTICES_T(v, graph1, Graph1) {
            cout << "  H vertex " << v 
                 << " -> G vertex " << get(f, v) << endl;
        }
        return true; // continue searching
    }

    const Graph1& graph1;
    const Graph2& graph2;
};

int main() {
    // Define graph type
    typedef adjacency_list<vecS, vecS, undirectedS> Graph;

    // Target graph G (larger graph)
    Graph G(6);
    add_edge(0, 1, G);
    add_edge(1, 2, G);
    add_edge(2, 3, G);
    add_edge(3, 4, G);
    add_edge(4, 5, G);
    add_edge(1, 4, G); // extra edge

    // Query graph H (smaller graph)
    Graph H(3);
    add_edge(0, 1, H);
    add_edge(1, 2, H);

    // Run VF2 subgraph isomorphism
    my_callback<Graph, Graph> callback(H, G);
    vf2_subgraph_iso(H, G, callback);

    return 0;
}
