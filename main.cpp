#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

#include <lemon/static_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/cost_scaling.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cycle_canceling.h>
#include <lemon/lgf_writer.h>

#include<ctime>
#include<sys/timeb.h>
#include<cinttypes>

using namespace lemon;
using namespace std;

struct Server {
    vector<unsigned int> consumption_rate;
    unsigned int transition_cost;

    Server(vector<unsigned int> cr, unsigned int tc) {
        consumption_rate = move(cr);
        transition_cost = tc;
    }
};

void generate_graph(SmartDigraph &g, SmartDigraph::NodeMap<long> &supply, SmartDigraph::ArcMap<int> &capacity,
                    SmartDigraph::ArcMap<int> &cost, vector<Server> &servers, vector<unsigned int> &demands) {

    vector<SmartDigraph::Node> sources;
    vector<SmartDigraph::Node> sinks;
    SmartDigraph::Node a_0 = g.addNode();
    supply.set(a_0, servers.size());
    sources.push_back(a_0);
    SmartDigraph::Node b_0 = g.addNode();
    supply.set(b_0, -1 * servers.size());
    sinks.push_back(b_0);

    for (auto it_demands = demands.begin(); it_demands != demands.end(); ++it_demands) {
        SmartDigraph::Node a_k = g.addNode();
        supply.set(a_k, *it_demands);
        sources.push_back(a_k);

        SmartDigraph::Node b_k = g.addNode();
        supply.set(b_k, -1 * *it_demands);
        sinks.push_back(b_k);
    }

    for (auto it_servers = servers.begin(); it_servers != servers.end(); ++it_servers) {
        SmartDigraph::Node u_1 = g.addNode();
        SmartDigraph::Node l_1 = g.addNode();

        for (int i = 0; i != demands.size(); ++i) {
            SmartDigraph::Node l_1a = g.addNode();
            SmartDigraph::Node l_1b = g.addNode();
            SmartDigraph::Node u_2 = g.addNode();
            SmartDigraph::Node l_2 = g.addNode();

            if (i == 0) {
                SmartDigraph::Arc edge = g.addArc(sources[0], l_1);
                cost.set(edge, 0);
                capacity.set(edge, 1);
            } else {
                SmartDigraph::Arc edge = g.addArc(u_1, l_1);
                cost.set(edge, 0);
                capacity.set(edge, 1);
            }

            SmartDigraph::Arc edge = g.addArc(u_1, u_2);
            cost.set(edge, it_servers->consumption_rate[i]);
            capacity.set(edge, 1);

            edge = g.addArc(l_1, l_1a);
            cost.set(edge, 0);
            capacity.set(edge, 1);

            edge = g.addArc(l_1, u_1);
            cost.set(edge, it_servers->transition_cost);
            capacity.set(edge, 1);

            edge = g.addArc(sources[i + 1], l_1a);
            cost.set(edge, 0);
            capacity.set(edge, 1);

            edge = g.addArc(l_1a, l_1b);
            cost.set(edge, 0);
            capacity.set(edge, 1);

            edge = g.addArc(l_1b, l_2);
            cost.set(edge, 0);
            capacity.set(edge, 1);

            edge = g.addArc(l_1b, sinks[i + 1]);
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
}

uint64_t getTimeNow() {
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t) ts.tv_sec * 1000000LL + (uint64_t) ts.tv_nsec / 1000LL;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

void read_test_file(const string &name, vector<Server> &servers, vector<unsigned int> &demands) {
    string line;
    ifstream file(name);
    if (file.is_open()) {
        getline(file, line);
        vector<string> buffer = split(line, ' ');
        int amount_servers = stoi(buffer[0]);
        int amount_demands = stoi(buffer[1]);

        getline(file, line);
        buffer = split(line, ' ');
        for (auto it = buffer.begin(); it != buffer.end(); ++it) {
            if (!(*it).empty()) {
                demands.push_back(stoi(*it));
            }
        }
        for (int i = 0; i != amount_servers; ++i) {
            getline(file, line);
            buffer = split(line, ' ');
            vector<unsigned int> consumption_rate;
            int transition_cost = stoi(buffer[0]);
            for (int i = 1; i != amount_demands + 1; ++i) {
                if (!buffer.empty()) {
                    consumption_rate.push_back(stoi(buffer[i]));
                }
            }
            servers.emplace_back(Server(consumption_rate, transition_cost));
        }
        file.close();
    } else cout << "Unable to open file " << name;
}

void benchmark(bool is_debug, int filenumber, uint64_t &generate, uint64_t &flow, string output) {
    srand(time(NULL));
    vector<Server> servers;
    vector<unsigned int> demands;
    read_test_file("tests/test_" + to_string(filenumber), servers, demands);
    ofstream file;
    file.open(output, std::ios_base::app);
    const long amount_nodes = 2 * (demands.size() + 1) + servers.size() * (4 * demands.size() + 2);
    const long amount_edges = servers.size() * (2 + 8 * demands.size());
    uint64_t start = getTimeNow();

    SmartDigraph g;
    SmartDigraph::NodeMap<long> supply(g);
    SmartDigraph::ArcMap<int> capacity(g);
    SmartDigraph::ArcMap<int> cost(g);

    generate_graph(g, supply, capacity, cost, servers, demands);
    uint64_t start_flow = getTimeNow();
    NetworkSimplex<SmartDigraph, int, int> alg(g);
    alg.costMap(cost);
    alg.supplyMap(supply);
    alg.upperMap(capacity);
    NetworkSimplex<SmartDigraph>::ProblemType ret = alg.run();
    uint64_t end = getTimeNow();

    switch (ret) {
        case NetworkSimplex<SmartDigraph>::INFEASIBLE:
            std::cerr << "INFEASIBLE" << std::endl;
            break;

        case NetworkSimplex<SmartDigraph>::OPTIMAL:
            std::cerr << "OPTIMAL" << std::endl;
            break;

        case NetworkSimplex<SmartDigraph>::UNBOUNDED:
            std::cerr << "UNBOUNDED" << std::endl;
    }

    uint64_t generate_bench = (start_flow - start);
    uint64_t flow_bench = (end - start_flow);
    uint64_t bench = generate_bench + flow_bench;
    generate += generate_bench;
    flow += flow_bench;
    printf("Generating the graph took %" PRIu64 " nanoseconds.\n", generate_bench);
    printf("Executing the simplex algorithm took %" PRIu64 " nanoseconds.\n", flow_bench);
    printf("Overall time required was %" PRIu64 " nanoseconds.\n", bench);
    file << (filenumber + 1) << " & " << amount_nodes << " & " << amount_edges << " & ";
    file << "\\SI{" << generate_bench << "}{\\nano\\second} & " << "\\SI{" << flow_bench << "}{\\nano\\second} & "
         << "\\SI{" << bench << "}{\\nano\\second}\\\\" << std::endl;
    file << "\\hline" << std::endl;
    file.close();

    if (is_debug) {
        SmartDigraph::ArcMap<int> flow(g);
        alg.flowMap(flow);
        digraphWriter(g).
                nodeMap("supply", supply).
                arcMap("cost", cost).
                arcMap("flow", flow).
                arcMap("capacity", capacity).
                run();
    }

}

int main() {
    const int amount_tests = 10;
    uint64_t generate = 0;
    uint64_t flow = 0;
    for (int i = 0; i != amount_tests + 1; ++i) {
        printf("running test %d\n", i);
        benchmark(false, i, generate, flow, "result");
    }
    printf("Ran %d tests.\n", amount_tests);
    printf("Overall time required for generating the graph: %" PRIu64 " nanoseconds\n", generate);
    printf("Overall time required for executing the simplex algorithm: %" PRIu64 " nanoseconds\n", flow);
    printf("Overall time required: %" PRIu64 " nanoseconds\n", generate + flow);
    return 0;
}


