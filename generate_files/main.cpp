#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace std;

void generate_test(int testnumber, int max_servers, int max_demands, int max_transition, int max_consumption) {
    ofstream file;
    file.open("tests/test_" + to_string(testnumber));
    srand(time(NULL));
    int amount_servers = max_servers;
    int amount_demands = max_demands;

    file << amount_servers << " " << amount_demands << std::endl;
    for(int i = 0; i != amount_demands; ++i) {
        int demand = rand() % (amount_servers + 1);
        file << demand << " ";
    }
    file << std::endl;
    for(int i = 0; i != amount_servers; ++i) {
        int transition_cost = (rand() % max_transition) + 1;
        file << transition_cost;
        for(int j = 0; j != amount_demands; ++j) {
            int consumption_rate = (rand() % max_consumption) + 1;
            file << " " << consumption_rate;
        }
        file << std::endl;
    }

    printf("Creating an test instance with %d servers and %d timesteps; max transition cost %d; max consumption rate: %d", amount_servers, amount_demands, max_transition, max_consumption);
    file.close();

}

int main(int argc, char * argv[]) {
    string testnumber = argv[1];
    string amount_servers = argv[2];
    string amount_demands = argv[3];
    string max_transition = argv[4];
    string max_consumption = argv[5];
    generate_test(stoi(testnumber), stoi(amount_servers), stoi(amount_demands), stoi(max_transition), stoi(max_consumption));
    return 0;
}

