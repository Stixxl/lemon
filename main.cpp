#include <iostream>
#include <vector>
#include <lemon/static_graph.h>
#include <lemon/smart_graph.h>
using namespace lemon;
using namespace std;

struct Server {
    unsigned long consumption_rate;
    unsigned long transition_cost;
};

StaticDigraph generate_graph(vector<Server> &servers, vector<unsigned int> &demands) {
    unsigned long size = (4 * demands.size() + 2) * servers.size() + (demands.size() + 1) * 2 + 2;
    int max_flow = servers.size();
    unsigned int timesteps = demands.size();
    SmartDigraph g;
    SmartDigraph::NodeMap<unsigned int> supply(g);
    SmartDigraph::ArcMap<int> capacity(g);
    SmartDigraph::ArcMap<int> cost(g);

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

         for(int i = 1; i != demands.size(); ++i) {
             SmartDigraph::Node l_1a = g.addNode();
             SmartDigraph::Node l_1b  = g.addNode();
             SmartDigraph::Node u_2 = g.addNode();
             SmartDigraph::Node l_2 = g.addNode();

             if(i == 1) {
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

             edge = g.addArc(sources[i], l_1a);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1a, l_1b);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1b, l_2);
             cost.set(edge, 0);
             capacity.set(edge, 1);

             edge = g.addArc(l_1b, sinks[i]);
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
int main()
{
    return 0;
}

