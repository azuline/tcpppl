#include <cassert>
#include <cstdio>
#include <deque>
#include <format>
#include <fstream>
#include <iomanip>
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

class Computer {
  private:
    Program p;
    int pc = 0;
    bool halted = false;

  public:
    Computer(Program p) : p(p) {};

    /**
     * Takes an input and executes until another input is expected or the program
     * halts. Returns a tuple containing the output so far and whethr the computer
     * halted.
     */
    std::tuple<std::deque<int>, bool> run(std::deque<int> &input) {
        assert(!halted);
        std::deque<int> output{};

        std::cerr << std::format("program state: pc={} memory=", pc);
        for (auto x : p.memory)
            std::cerr << x << " ";
        std::cerr << std::endl;

        while (true) {
            std::cerr << std::left << std::setw(36) << std::format("executing opcode memory[{}]={}", pc, p.memory[pc]) << " | ";
            assert(pc < p.memory.size());
            auto in = Instruction::parse(p.memory[pc]);

            switch (in.opcode) {
                case Opcode::halt: {
                    std::cerr << "halt" << std::endl;
                    halted = true;
                    return {output, true};
                }
                case Opcode::add: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    auto arg3 = p.memory[pc + 3];
                    std::cerr << std::format("*{} = {} + {}", arg3, arg1, arg2) << std::endl;
                    p.memory[arg3] = arg1 + arg2;
                    pc += 4;
                    break;
                }
                case Opcode::mul: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    auto arg3 = p.memory[pc + 3];
                    std::cerr << std::format("*{} = {} * {}", arg3, arg1, arg2) << std::endl;
                    p.memory[arg3] = arg1 * arg2;
                    pc += 4;
                    break;
                }
                case Opcode::input: {
                    if (input.empty()) {
                        std::cerr << "break" << std::endl;
                        return {output, false};
                    }
                    auto arg1 = p.memory[pc + 1];
                    auto arg2 = input.front();
                    std::cerr << std::format("*{} = {}", arg1, arg2) << std::endl;
                    p.memory[arg1] = arg2;
                    pc += 2;
                    input.pop_front();
                    break;
                }
                case Opcode::output: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    std::cerr << std::format("print({})", arg1) << std::endl;
                    output.push_back(arg1);
                    pc += 2;
                    break;
                }
                case Opcode::jump_true: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    std::cerr << std::format("pc = {} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 != 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::jump_false: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    std::cerr << std::format("pc = !{} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 == 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::less_than: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    auto arg3 = p.memory[pc + 3];
                    std::cerr << std::format("*{} = {} < {}", arg3, arg1, arg2) << std::endl;
                    p.memory[arg3] = arg1 < arg2 ? 1 : 0;
                    pc += 4;
                    break;
                }
                case Opcode::equals: {
                    auto arg1 = eval_argument(p, p.memory[pc + 1], in.mode1);
                    auto arg2 = eval_argument(p, p.memory[pc + 2], in.mode2);
                    auto arg3 = p.memory[pc + 3];
                    std::cerr << std::format("*{} = {} == {}", arg3, arg1, arg2) << std::endl;
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
    std::vector<int> signals{0, 1, 2, 3, 4};
    int largest_thruster{0};
    do {
        std::array<Computer, 5> amps{{Computer(program), Computer(program), Computer(program), Computer(program), Computer(program)}};
        std::deque<int> prev_amp_out{0};
        for (auto i = 0; i < 5; i++) {
            auto amp = &amps[i];
            auto amp_in = prev_amp_out;
            amp_in.push_front(signals[i]);
            auto [out, halted] = amp->run(amp_in);
            prev_amp_out = out;
        }
        largest_thruster = std::max(largest_thruster, prev_amp_out[0]);
    } while (std::next_permutation(signals.begin(), signals.end()));

    std::cout << std::format("Part 1: {}\n", largest_thruster);
};

void part2(Program program) {
    std::vector<int> signals{5, 6, 7, 8, 9};
    int largest_thruster{0};
    do {
        std::array<Computer, 5> amps{{Computer(program), Computer(program), Computer(program), Computer(program), Computer(program)}};
        std::array<std::deque<int>, 5> inputs{{{signals[0], 0}, {signals[1]}, {signals[2]}, {signals[3]}, {signals[4]}}};
        int thruster;
        while (true) {
            for (auto i = 0; i < 5; i++) {
                assert(!inputs[i].empty());
                std::cerr << std::format("running amp {} with inputs ", i);
                for (auto x : inputs[i])
                    std::cerr << x << " ";
                std::cerr << std::endl;
                auto amp = &amps[i];
                auto [out, halted] = amp->run(inputs[i]);
                auto next_input = &inputs[(i + 1) % 5];
                for (auto x : out)
                    next_input->push_back(x);
                if (i == 4 && halted) {
                    thruster = out[0];
                    goto end;
                }
            }
        }
    end:
        largest_thruster = std::max(largest_thruster, thruster);
    } while (std::next_permutation(signals.begin(), signals.end()));

    std::cout << std::format("Part 1: {}", largest_thruster) << std::endl;
};

// clang-format off
const std::string TEST_INPUT = "3,26,1001,26,-4,26,3,27,1002,27,2,27,1,27,26,27,4,27,1001,28,-1,28,1005,28,6,99,0,0,5";
// clang-format on

int main() {
    std::ifstream real_input("inputs/day07.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);
    part2(program);

    return 0;
}
