#include <cassert>
#include <cstddef>
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
    relative_base = 9,
    halt = 99,
};

enum class ParamMode {
    position = 0,
    immediate = 1,
    relative = 2,
};

typedef long code;

class Program {
  private:
    std::vector<code> memory;
    Program(std::vector<code> memory) : memory(memory) {};

  public:
    static Program parse(std::istream &input_stream) {
        std::vector<code> program{};
        for (std::string opcode_s; std::getline(input_stream, opcode_s, ',');) {
            auto opcode = std::stol(opcode_s);
            program.push_back(opcode);
        }
        return Program(program);
    }
    code read(std::size_t index) {
        return index < memory.size() ? memory[index] : 0;
    }
    void write(std::size_t index, code value) {
        if (index >= memory.size())
            memory.resize(index + 1);
        memory[index] = value;
    }
};

class Instruction {
  public:
    Opcode opcode;
    ParamMode mode1;
    ParamMode mode2;
    ParamMode mode3;
    static Instruction parse(code x) {
        return Instruction(Opcode(x % 100), ParamMode((x / 100) % 10), ParamMode((x / 1000) % 10), ParamMode((x / 10000) % 10));
    };
};

class Computer {
  private:
    Program p;
    code pc = 0;
    code relative_base = 0;
    bool halted = false;
    code eval_read_operand(code parameter, ParamMode mode) {
        switch (mode) {
            case ParamMode::position:
                return p.read(parameter);
            case ParamMode::immediate:
                return parameter;
            case ParamMode::relative:
                return p.read(relative_base + parameter);
            default:
                throw std::invalid_argument(std::format("{} is not a supported parameter mode", static_cast<int>(mode)));
        }
    }
    code eval_write_operand(code parameter, ParamMode mode) {
        switch (mode) {
            case ParamMode::position:
                return parameter;
            case ParamMode::relative:
                return relative_base + parameter;
            case ParamMode::immediate:
                throw new std::invalid_argument("write operands cannot be in immediate mode");
            default:
                throw std::invalid_argument(std::format("{} is not a supported parameter mode", static_cast<int>(mode)));
        }
    }

  public:
    Computer(Program p) : p(p) {};

    /**
     * Takes an input and executes until another input is expected or the program
     * halts. Returns a tuple containing the output so far and whethr the computer
     * halted.
     */
    std::tuple<std::deque<code>, bool> run(std::deque<code> &input) {
        assert(!halted);
        std::deque<code> output{};

        while (true) {
            std::cerr << std::left << std::setw(36) << std::format("executing opcode memory[{}]={}", pc, p.read(pc)) << " | ";
            auto in = Instruction::parse(p.read(pc));

            switch (in.opcode) {
                case Opcode::halt: {
                    std::cerr << "halt" << std::endl;
                    halted = true;
                    return {output, true};
                }
                case Opcode::add: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    auto arg3 = eval_write_operand(p.read(pc + 3), in.mode3);
                    std::cerr << std::format("*{} = {} + {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 + arg2);
                    pc += 4;
                    break;
                }
                case Opcode::mul: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    auto arg3 = eval_write_operand(p.read(pc + 3), in.mode3);
                    std::cerr << std::format("*{} = {} * {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 * arg2);
                    pc += 4;
                    break;
                }
                case Opcode::input: {
                    if (input.empty()) {
                        std::cerr << "break" << std::endl;
                        return {output, false};
                    }
                    auto arg1 = eval_write_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = input.front();
                    std::cerr << std::format("*{} = {}", arg1, arg2) << std::endl;
                    p.write(arg1, arg2);
                    pc += 2;
                    input.pop_front();
                    break;
                }
                case Opcode::output: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    std::cerr << std::format("print({})", arg1) << std::endl;
                    output.push_back(arg1);
                    pc += 2;
                    break;
                }
                case Opcode::jump_true: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    std::cerr << std::format("pc = {} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 != 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::jump_false: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    std::cerr << std::format("pc = !{} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 == 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::less_than: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    auto arg3 = eval_write_operand(p.read(pc + 3), in.mode3);
                    std::cerr << std::format("*{} = {} < {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 < arg2 ? 1 : 0);
                    pc += 4;
                    break;
                }
                case Opcode::equals: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    auto arg2 = eval_read_operand(p.read(pc + 2), in.mode2);
                    auto arg3 = eval_write_operand(p.read(pc + 3), in.mode3);
                    std::cerr << std::format("*{} = {} == {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 == arg2 ? 1 : 0);
                    pc += 4;
                    break;
                }
                case Opcode::relative_base: {
                    auto arg1 = eval_read_operand(p.read(pc + 1), in.mode1);
                    std::cerr << std::format("rb += {}", arg1) << std::endl;
                    relative_base += arg1;
                    pc += 2;
                    break;
                }
                default: {
                    throw std::invalid_argument(std::format("memory[{}]={} is not a supported opcode", pc, p.read(pc)));
                }
            }
        }
    }
};

void part1(Program program) {
    auto computer = Computer(program);
    std::deque<code> input{1};
    auto [output, halted] = computer.run(input);
    std::cout << std::format("Part 1: {}\n", output[0]);
};

void part2(Program program) {
    auto computer = Computer(program);
    std::deque<code> input{2};
    auto [output, halted] = computer.run(input);
    std::cout << std::format("Part 1: {}\n", output[0]);
};

// clang-format off
// const std::string TEST_INPUT = "104,1125899906842624,99";
const std::string TEST_INPUT = "109,1,204,-1,1001,100,1,100,1008,100,16,101,1006,101,0,99";
// clang-format on

int main() {
    std::ifstream real_input("inputs/day09.txt");
    std::istringstream test_input{TEST_INPUT};
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);
    part2(program);

    return 0;
}
