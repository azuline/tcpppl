#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class Opcode {
    add = 1,
    mul = 2,
    input = 3,
    output = 4,
    jump_true = 5,
    jump_false = 6,
    less_than = 7,
    equals = 8,
    halt = 99,
};

enum class ParamMode {
    position = 0,
    immediate = 1,
    output = 2,
};

class Instruction {
  private:
    Instruction(Opcode opcode, ParamMode mode1, ParamMode mode2, ParamMode mode3)
        : opcode(opcode), mode1(mode1), mode2(mode2), mode3(mode3) {};

  public:
    Opcode opcode;
    ParamMode mode1;
    ParamMode mode2;
    ParamMode mode3;
    static Instruction parse(int x) {
        return Instruction(
                Opcode(x % 100), ParamMode((x / 100) % 10), ParamMode((x / 1000) % 10), ParamMode((x / 10000) % 10));
    };
};

class Program {
  private:
    Program(std::vector<int> memory) : memory(memory) {};

  public:
    std::vector<int> memory;
    static Program parse(std::istream &input_stream) {
        std::vector<int> program{};
        for (std::string opcode_s; std::getline(input_stream, opcode_s, ',');) {
            auto opcode = std::stoi(opcode_s);
            program.push_back(opcode);
        }
        return Program(program);
    }
};

class Argument {
  public:
    int parameter;
    ParamMode mode;
    int eval(Program &program) {
        switch (mode) {
            case ParamMode::immediate:
            case ParamMode::output:
                return parameter;
            case ParamMode::position:
                return program.memory[parameter];
            default:
                throw std::invalid_argument(
                        std::format("{} is not a supported parameter mode", static_cast<int>(mode)));
        }
    }
};

int computer(Program p) {
    int pc = 0;
    while (true) {
        std::fprintf(stderr, "executing opcode memory[%d]=%d\n", pc, p.memory[pc]);
        assert(pc < p.memory.size());
        auto in = Instruction::parse(p.memory[pc]);

        if (in.opcode == Opcode::halt) {
            break;
        } else if (in.opcode == Opcode::add) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            auto arg3 = Argument(p.memory[pc + 3], ParamMode::output).eval(p);
            std::fprintf(stderr, "    *%d = %d + %d\n", arg3, arg1, arg2);
            p.memory[arg3] = arg1 + arg2;
            pc += 4;
        } else if (in.opcode == Opcode::mul) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            auto arg3 = Argument(p.memory[pc + 3], ParamMode::output).eval(p);
            std::fprintf(stderr, "    *%d = %d * %d\n", arg3, arg1, arg2);
            p.memory[arg3] = arg1 * arg2;
            pc += 4;
        } else if (in.opcode == Opcode::input) {
            auto arg1 = Argument(p.memory[pc + 1], ParamMode::output).eval(p);
            std::printf("Awaiting your input: ");
            std::string input;
            std::getline(std::cin, input);
            auto arg2 = std::stoi(input);
            std::fprintf(stderr, "    *%d = %d\n", arg1, arg2);
            p.memory[arg1] = arg2;
            pc += 2;
        } else if (in.opcode == Opcode::output) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            std::fprintf(stderr, "    print(%d)\n", arg1);
            std::printf("Writing output: %d\n", arg1);
            pc += 2;
        } else if (in.opcode == Opcode::jump_true) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            std::fprintf(stderr, "    pc = %d ? %d : pc+3\n", arg1, arg2);
            pc = arg1 != 0 ? arg2 : pc + 3;
        } else if (in.opcode == Opcode::jump_false) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            std::fprintf(stderr, "    pc = !%d ? %d : pc+3\n", arg1, arg2);
            pc = arg1 == 0 ? arg2 : pc + 3;
        } else if (in.opcode == Opcode::less_than) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            auto arg3 = Argument(p.memory[pc + 3], ParamMode::output).eval(p);
            std::fprintf(stderr, "    *%d = %d < %d\n", arg3, arg1, arg2);
            p.memory[arg3] = arg1 < arg2 ? 1 : 0;
            pc += 4;
        } else if (in.opcode == Opcode::equals) {
            auto arg1 = Argument(p.memory[pc + 1], in.mode1).eval(p);
            auto arg2 = Argument(p.memory[pc + 2], in.mode2).eval(p);
            auto arg3 = Argument(p.memory[pc + 3], ParamMode::output).eval(p);
            std::fprintf(stderr, "    *%d = %d == %d\n", arg3, arg1, arg2);
            p.memory[arg3] = arg1 == arg2 ? 1 : 0;
            pc += 4;
        } else {
            throw std::invalid_argument(std::format("memory[{}]={} is not a supported opcode", pc, p.memory[pc]));
        }
    }
    return p.memory[0];
};

void part1(Program program) {
    std::printf("Part 1:\n");
    computer(program);
}

void part2(Program program) {
    std::printf("Part 2:\n");
    computer(program);
}

// clang-format off
const std::string TEST_INPUT = "3,21,1008,21,8,20,1005,20,22,107,8,21,20,1006,20,31,1106,0,36,98,0,0,1002,21,125,20,4,20,1105,1,46,104,999,1105,1,46,1101,1000,1,20,4,20,1105,1,46,98,99";
// clang-format on

int main() {
    std::ifstream real_input("inputs/day05.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);
    part2(program);

    return 0;
}
