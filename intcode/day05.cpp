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
};

class Program {
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

class Instruction {
  public:
    Opcode opcode;
    ParamMode mode1;
    ParamMode mode2;
    ParamMode mode3;
    static Instruction parse(int x) {
        return Instruction(Opcode(x % 100), ParamMode((x / 100) % 10), ParamMode((x / 1000) % 10), ParamMode((x / 10000) % 10));
    };
};

int eval_argument(Program &p, int parameter, ParamMode mode) {
    switch (mode) {
        case ParamMode::position:
            return p.memory[parameter];
        case ParamMode::immediate:
            return parameter;
        default:
            throw std::invalid_argument(std::format("{} is not a supported parameter mode", static_cast<int>(mode)));
    }
}

int computer(Program p) {
    int pc = 0;

    while (true) {
        std::fprintf(stderr, "executing opcode memory[%d]=%d\n", pc, p.memory[pc]);
        assert(pc < p.memory.size());
        auto in = Instruction::parse(p.memory[pc]);

        switch (in.opcode) {
            case Opcode::halt: {
                goto terminate;
            }
            case Opcode::add: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                auto arg3 = p.memory[pc + 3];
                std::fprintf(stderr, "    *%d = %d + %d\n", arg3, arg1, arg2);
                p.memory[arg3] = arg1 + arg2;
                pc += 4;
                break;
            }
            case Opcode::mul: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                auto arg3 = p.memory[pc + 3];
                std::fprintf(stderr, "    *%d = %d * %d\n", arg3, arg1, arg2);
                p.memory[arg3] = arg1 * arg2;
                pc += 4;
                break;
            }
            case Opcode::input: {
                auto arg1 = p.memory[pc + 1];
                std::printf("Awaiting your input: ");
                std::string input;
                std::getline(std::cin, input);
                auto arg2 = std::stoi(input);
                std::fprintf(stderr, "    *%d = %d\n", arg1, arg2);
                p.memory[arg1] = arg2;
                pc += 2;
                break;
            }
            case Opcode::output: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                std::fprintf(stderr, "    print(%d)\n", arg1);
                std::printf("Writing output: %d\n", arg1);
                pc += 2;
                break;
            }
            case Opcode::jump_true: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                std::fprintf(stderr, "    pc = %d ? %d : pc+3\n", arg1, arg2);
                pc = arg1 != 0 ? arg2 : pc + 3;
                break;
            }
            case Opcode::jump_false: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                std::fprintf(stderr, "    pc = !%d ? %d : pc+3\n", arg1, arg2);
                pc = arg1 == 0 ? arg2 : pc + 3;
                break;
            }
            case Opcode::less_than: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                auto arg3 = p.memory[pc + 3];
                std::fprintf(stderr, "    *%d = %d < %d\n", arg3, arg1, arg2);
                p.memory[arg3] = arg1 < arg2 ? 1 : 0;
                pc += 4;
                break;
            }
            case Opcode::equals: {
                auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                auto arg3 = p.memory[pc + 3];
                std::fprintf(stderr, "    *%d = %d == %d\n", arg3, arg1, arg2);
                p.memory[arg3] = arg1 == arg2 ? 1 : 0;
                pc += 4;
                break;
            }
            default: {
                throw std::invalid_argument(std::format("memory[{}]={} is not a supported opcode", pc, p.memory[pc]));
            }
        }
    }
terminate:
    return p.memory[0];
};

// clang-format off
const std::string TEST_INPUT = "3,21,1008,21,8,20,1005,20,22,107,8,21,20,1006,20,31,1106,0,36,98,0,0,1002,21,125,20,4,20,1105,1,46,104,999,1105,1,46,1101,1000,1,20,4,20,1105,1,46,98,99";
// clang-format on

int main() {
    std::ifstream real_input("inputs/day05.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    computer(program);

    return 0;
}
