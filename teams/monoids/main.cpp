#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <queue>

enum class ModuleState {
    ANY = 0,
    ON,
    OFF
};

enum class ReqType {
    DEP,
    CONFLICT
};

struct Module {
    ModuleState state;
    std::vector<int> deps;
    std::vector<int> conflicts;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Wrong amount of arguments" << std::endl;
        return -1;
    }

    std::ifstream in(argv[1]);

    int modules_amount, reqs_amount;

    in >> modules_amount >> reqs_amount;

    std::vector<Module> modules(modules_amount);
    modules.resize(modules_amount);

    std::queue<int> analyzeQueue;

    std::string directive;
    for (int i = 0; i < reqs_amount; i++) {
        in >> directive;

        if (directive == "DEP") {
            int A, B;
            in >> A >> B;
            A--;
            B--;
            modules[A].deps.push_back(B);
        }
        else if (directive == "CONFLICT") {
            int A, B;
            in >> A >> B;
            A--;
            B--;
            modules[A].conflicts.push_back(B);
            modules[B].conflicts.push_back(A);
        }
        else if (directive == "REQUIRE") {
            int index;
            in >> index;
            index--;
            modules[index].state = ModuleState::ON;
            analyzeQueue.push(index);
        }
        else {
            std::cout << "Unknown directive: " << directive << std::endl;
            return -1;
        }
    }

    while (!analyzeQueue.empty()) {
        int index = analyzeQueue.front();
        analyzeQueue.pop();
        
        if (modules[index].state == ModuleState::ON) {
            for (int dep : modules[index].deps) {
                if (modules[dep].state == ModuleState::OFF) {
                    std::cout << "IMPOSSIBLE" << std::endl;
                    return 0;
                }
                else if (modules[dep].state == ModuleState::ANY) {
                    modules[dep].state = ModuleState::ON;
                    analyzeQueue.push(dep);
                }
            }
            for (int con : modules[index].conflicts) {
                if (modules[con].state == ModuleState::ON) {
                    std::cout << "IMPOSSIBLE" << std::endl;
                    return 0;
                }
                else if (modules[con].state == ModuleState::ANY) {
                    modules[con].state = ModuleState::OFF;
                    analyzeQueue.push(con);
                }
            }
        }
    }

    for (int i = 0; i < modules.size(); i++) {
        std::cout << i + 1 << ": ";
        switch (modules[i].state){
            case ModuleState::ANY:
            case ModuleState::ON:
                std::cout << "ON" << std::endl;
                break;
            case ModuleState::OFF:
                std::cout << "OFF" << std::endl;
                break;
        }
    }

    in.close();

    return 0;
}
