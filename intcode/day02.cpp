#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

const int ADD_CODE = 1;
const int MUL_CODE = 2;
const int HALT_CODE = 99;

typedef std::vector<int> program;

program parse_program(std::istream &input_stream) {
    // Parse the program.
    std::vector<int> program{};
    for (std::string opcode_s; std::getline(input_stream, opcode_s, ',');) {
        auto opcode = std::stoi(opcode_s);
        program.push_back(opcode);
    }
    return program;
}

void part1(program program) {
    // Initial program modifications.
    program[1] = 12;
    program[2] = 2;

    // Evaluate the program.
    int pc = 0;
    while (program[pc] != HALT_CODE) {
        std::fprintf(stderr, "executing opcode program[%d]=%d\n", pc, program[pc]);
        if (program[pc] == ADD_CODE) {
            auto arg1 = program[program[pc + 1]];
            auto arg2 = program[program[pc + 2]];
            std::fprintf(stderr, "    addition: %d+%d >> *%d\n", arg1, arg2, program[pc + 3]);
            program[program[pc + 3]] = arg1 + arg2;
            pc += 4;
        } else if (program[pc] == MUL_CODE) {
            auto arg1 = program[program[pc + 1]];
            auto arg2 = program[program[pc + 2]];
            std::fprintf(stderr, "    multiplication: %d*%d >> *%d\n", arg1, arg2, program[pc + 3]);
            program[program[pc + 3]] = arg1 * arg2;
            pc += 4;
        } else {
            throw std::invalid_argument(std::format("program[{}]={} is not a supported opcode", pc, program[pc]));
        }
        std::fprintf(stderr, "    pc = %d\n", pc);
    }

    std::printf("Part 1: %d", program[0]);
}

const std::string TEST_INPUT = "1,9,10,3,2,3,11,0,99,30,40,50";

int main() {
    std::ifstream real_input("inputs/day02.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    program program = parse_program(*input);
    part1(program);

    return 0;
}
