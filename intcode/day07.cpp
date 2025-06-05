#include <cassert>
#include <cstdio>
#include <deque>
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

class Pipe {
  private:
    std::deque<int> data;
    std::vector<Pipe> sinks;

  public:
    int read() {
        int r = this->data.front();
        this->data.pop_front();
        return r;
    }
    void write(int value) {
        this->data.push_back(value);
        int r = this->data.front();
    }
    void connect(Pipe io) {
        this->sinks.push_back(io);
    }
};

class Computer {
  private:
    Program p;
    int pc = 0;
    bool halted = false;

  public:
    Computer(Program p) : p(p) {};

    /**
     * Takes an input and executes until another input is expected or the program halts.
     * Returns a tuple containing the output so far and whethr the computer halted.
     */
    std::tuple<std::deque<int>, bool> run(std::deque<int> input) {
        std::deque<int> output{};

        while (true) {
            std::fprintf(stderr, "executing opcode memory[%d]=%d\n", pc, p.memory[pc]);
            assert(pc < p.memory.size());
            auto in = Instruction::parse(p.memory[pc]);

            switch (in.opcode) {
                case Opcode::halt: {
                    return {output, true};
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
                    if (input.empty()) {
                        return {output, false};
                    }
                    auto arg1 = p.memory[pc + 1];
                    auto arg2 = input.front();
                    std::fprintf(stderr, "    *%d = %d\n", arg1, arg2);
                    p.memory[arg1] = arg2;
                    pc += 2;
                    input.pop_front();
                    break;
                }
                case Opcode::output: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    std::fprintf(stderr, "    print(%d)\n", arg1);
                    output.push_back(arg1);
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
    }
};

void part1(Program program) {
    Computer amps[5] = {Computer(program), Computer(program), Computer(program), Computer(program), Computer(program)};

    std::vector<int> phase_signals{0, 1, 2, 3, 4};
    int largest_thruster{0};
    do {
        std::deque<int> prev_amp_out{0};
        for (auto i = 0; i < 5; i++) {
            auto amp = amps[i];
            auto amp_in = prev_amp_out;
            amp_in.push_front(phase_signals[i]);
            auto [out, halted] = amp.run(amp_in);
            prev_amp_out = out;
        }
        largest_thruster = std::max(largest_thruster, prev_amp_out[0]);
    } while (std::next_permutation(phase_signals.begin(), phase_signals.end()));

    std::printf("Part 1: %d\n", largest_thruster);
};

// clang-format off
const std::string TEST_INPUT = "3,15,3,16,1002,16,10,16,1,16,15,15,4,15,99,0,0";
// clang-format on

int main() {
    std::ifstream real_input("inputs/day07.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);

    return 0;
}
