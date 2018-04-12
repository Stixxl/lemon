#include <iostream>
#include <fstream>

#include <vector>

#include <lemon/static_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/lgf_writer.h>
using namespace lemon;
using namespace std;

struct Server {
    unsigned long consumption_rate;
    unsigned long transition_cost;

    Server(unsigned long cr, unsigned long tc) {
        consumption_rate = cr;
        transition_cost = tc;
    }
};

void generate_graph(SmartDigraph &g, SmartDigraph::NodeMap<int> &supply, SmartDigraph::ArcMap<int> &capacity,
                    SmartDigraph::ArcMap<int> &cost, vector<Server> &servers, vector<unsigned int> &demands) {
    unsigned long size = (4 * demands.size() + 2) * servers.size() + (demands.size() + 1) * 2 + 2;
    int max_flow = servers.size();
    unsigned int timesteps = demands.size();
    //SmartDigraph::Node source = g.addNode();
    //SmartDigraph::Node sink = g.addNode();

    vector<SmartDigraph::Node> sources;
    vector<SmartDigraph::Node> sinks;
    SmartDigraph::Node a_0 = g.addNode();
    supply.set(a_0, servers.size());
    sources.push_back(a_0);
    SmartDigraph::Node b_0 = g.addNode();
    supply.set(b_0, -1 * servers.size());
    sinks.push_back(b_0);

    for(vector<unsigned int>::iterator it_demands = demands.begin(); it_demands != demands.end(); ++it_demands) {
        SmartDigraph::Node a_k = g.addNode();
        supply.set(a_k, *it_demands);
        sources.push_back(a_k);

        SmartDigraph::Node b_k = g.addNode();
        supply.set(b_k, -1 * *it_demands);
        sinks.push_back(b_k);
    }

     for(vector<Server>::iterator it_servers = servers.begin(); it_servers != servers.end(); ++it_servers) {
         SmartDigraph::Node u_1 = g.addNode();
         SmartDigraph::Node l_1 = g.addNode();

         for(int i = 0; i != demands.size(); ++i) {
             SmartDigraph::Node l_1a = g.addNode();
             SmartDigraph::Node l_1b  = g.addNode();
             SmartDigraph::Node u_2 = g.addNode();
             SmartDigraph::Node l_2 = g.addNode();

             if(i == 0) {
                 SmartDigraph::Arc edge = g.addArc(sources[0], l_1);
                 cost.set(edge, 0);
                 capacity.set(edge, 1);
             } else {
                 SmartDigraph::Arc edge = g.addArc(u_1, l_1);
                 cost.set(edge, 0);
                 capacity.set(edge, 1);
             }

             SmartDigraph::Arc edge = g.addArc(u_1, u_2);
             cost.set(edge, it_servers->consumption_rate);
             capacity.set(edge, 1);

             edge = g.addArc(l_1, l_1a);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1, u_1);
             cost.set(edge, it_servers->transition_cost);
             capacity.set(edge, 1);

             edge = g.addArc(sources[i+1], l_1a);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1a, l_1b);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1b, l_2);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1b, sinks[i+1]);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             u_1 = u_2;
             l_1 = l_2;

     }
        SmartDigraph::Arc edge = g.addArc(u_1, l_1);
         cost.set(edge, 0);
         capacity.set(edge, 1);

         edge = g.addArc(l_1, sinks[0]);
         cost.set(edge, 0);
         capacity.set(edge, 1);

    }
    /*SmartDigraph::Arc edge = g.addArc(source, a_0);
    capacity.set(edge, servers.size());
    cost.set(edge, 0);
    SmartDigraph::Arc edge = g.addArc(b_0, sink);
    capacity.set(edge, servers.size());*/
}
void print_graph(SmartDigraph &g, SmartDigraph::NodeMap<int> &supply) {
    ofstream file;
    file.open("dot.gv");
    file << "digraph G {" << std::endl;
    for (SmartDigraph::ArcIt a(g); a != INVALID; ++a) {
    file  << g.id(g.source(a)) << " -> " << g.id(g.target(a))
              << std::endl;
}
    file << "}" << std::endl;
}
int main()
{
    vector<Server> servers;
    servers.push_back(Server(1,2));
    servers.push_back(Server(3,4));
    vector<unsigned int> demands;
    demands.push_back(1);
    demands.push_back(2);


    SmartDigraph g;
    SmartDigraph::NodeMap<int> supply(g);
    SmartDigraph::ArcMap<int> capacity(g);
    SmartDigraph::ArcMap<int> cost(g);
    SmartDigraph::ArcMap<int> flow(g);

    generate_graph(g, supply, capacity, cost, servers, demands);
    print_graph(g, supply);
    NetworkSimplex<SmartDigraph, int, int> simplex(g);
    simplex.costMap(cost);
    simplex.supplyMap(supply);
    simplex.upperMap(capacity);
    simplex.flowMap(flow);
    NetworkSimplex<SmartDigraph>::ProblemType ret = simplex.run();

    switch ( ret )
    {
        case NetworkSimplex<SmartDigraph>::INFEASIBLE:
            std::cerr << "INFEASIBLE" << std::endl;
        break;

        case NetworkSimplex<SmartDigraph>::OPTIMAL:
            std::cerr << "OPTIMAL" << std::endl;
        break;

        case NetworkSimplex<SmartDigraph>::UNBOUNDED:
            std::cerr << "UNBOUNDED" << std::endl;
    }

    digraphWriter(g).
            nodeMap("supply", supply).      // write g to the standard output
            arcMap("cost", cost).          // write 'cost' for for arcs
            arcMap("flow", flow).
            arcMap("capacity", capacity).
            run();

    return 0;
}

