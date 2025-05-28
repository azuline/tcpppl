#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

const int ADD_CODE = 1;
const int MUL_CODE = 2;
const int INPUT_CODE = 3;
const int OUTPUT_CODE = 3;
const int HALT_CODE = 99;

class Program {
  private:
    Program(std::vector<int> memory) : memory(memory) {};
    std::vector<int> memory;

  public:
    static Program parse(std::istream &input_stream);
    int execute(std::tuple<int, int> inputs);
};

Program Program::parse(std::istream &input_stream) {
    std::vector<int> program{};
    for (std::string opcode_s; std::getline(input_stream, opcode_s, ',');) {
        auto opcode = std::stoi(opcode_s);
        program.push_back(opcode);
    }
    return Program(program);
}

int Program::execute(std::tuple<int, int> inputs) {
    memory[1] = get<0>(inputs);
    memory[2] = get<1>(inputs);

    int pc = 0;
    while (memory[pc] != HALT_CODE) {
        std::fprintf(stderr, "executing opcode memory[%d]=%d\n", pc, memory[pc]);
        if (memory[pc] == ADD_CODE) {
            auto arg1 = memory[memory[pc + 1]];
            auto arg2 = memory[memory[pc + 2]];
            std::fprintf(stderr, "    *%d=%d+%d\n", memory[pc + 3], arg1, arg2);
            memory[memory[pc + 3]] = arg1 + arg2;
            pc += 4;
        } else if (memory[pc] == MUL_CODE) {
            auto arg1 = memory[memory[pc + 1]];
            auto arg2 = memory[memory[pc + 2]];
            std::fprintf(stderr, "    *%d=%d*%d\n", memory[pc + 3], arg1, arg2);
            memory[memory[pc + 3]] = arg1 * arg2;
            pc += 4;
        } else {
            throw std::invalid_argument(std::format("memory[{}]={} is not a supported opcode", pc, memory[pc]));
        }
        std::fprintf(stderr, "    pc=%d\n", pc);
    }
    return memory[0];
}

void part1(Program program) {
    std::printf("Part 1: %d\n", program.execute({12, 2}));
}

void part2(Program program) {
    int solution;
    for (int noun = 0; noun < 100; noun++) {
        for (int verb = 0; verb < 100; verb++) {
            auto clone = program;
            int result;
            try {
                result = clone.execute({noun, verb});
            } catch (std::invalid_argument) {
                continue;
            }
            if (result == 19690720) {
                solution = 100 * noun + verb;
                break;
            }
        }
    }
    std::printf("Part 2: %d\n", solution);
}

const std::string TEST_INPUT = "1,9,10,3,2,3,11,0,99,30,40,50";

int main() {
    std::ifstream real_input("inputs/day02.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);
    part2(program);

    return 0;
}
