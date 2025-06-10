#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <deque>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <ranges>
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

enum class RWMode { read, write };

class Computer {
  private:
    Program p;
    code pc = 0;
    code relative_base = 0;
    bool halted = false;
    code eval_operand(code parameter, ParamMode mode, RWMode rw) {
        if (mode == ParamMode::immediate) {
            assert(rw == RWMode::read);
            return parameter;
        }

        code address;
        if (mode == ParamMode::position) {
            address = parameter;
        } else if (mode == ParamMode::relative) {
            address = relative_base + parameter;
        } else {
            throw std::invalid_argument(std::format("{} is not a supported parameter mode", static_cast<int>(mode)));
        }

        return rw == RWMode::read ? p.read(address) : address;
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
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    auto arg3 = eval_operand(p.read(pc + 3), in.mode3, RWMode::write);
                    std::cerr << std::format("*{} = {} + {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 + arg2);
                    pc += 4;
                    break;
                }
                case Opcode::mul: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    auto arg3 = eval_operand(p.read(pc + 3), in.mode3, RWMode::write);
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
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::write);
                    auto arg2 = input.front();
                    std::cerr << std::format("*{} = {}", arg1, arg2) << std::endl;
                    p.write(arg1, arg2);
                    pc += 2;
                    input.pop_front();
                    break;
                }
                case Opcode::output: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    std::cerr << std::format("print({})", arg1) << std::endl;
                    output.push_back(arg1);
                    pc += 2;
                    break;
                }
                case Opcode::jump_true: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    std::cerr << std::format("pc = {} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 != 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::jump_false: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    std::cerr << std::format("pc = !{} ? {} : pc+3", arg1, arg2) << std::endl;
                    pc = arg1 == 0 ? arg2 : pc + 3;
                    break;
                }
                case Opcode::less_than: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    auto arg3 = eval_operand(p.read(pc + 3), in.mode3, RWMode::write);
                    std::cerr << std::format("*{} = {} < {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 < arg2 ? 1 : 0);
                    pc += 4;
                    break;
                }
                case Opcode::equals: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
                    auto arg2 = eval_operand(p.read(pc + 2), in.mode2, RWMode::read);
                    auto arg3 = eval_operand(p.read(pc + 3), in.mode3, RWMode::write);
                    std::cerr << std::format("*{} = {} == {}", arg3, arg1, arg2) << std::endl;
                    p.write(arg3, arg1 == arg2 ? 1 : 0);
                    pc += 4;
                    break;
                }
                case Opcode::relative_base: {
                    auto arg1 = eval_operand(p.read(pc + 1), in.mode1, RWMode::read);
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

enum class Direction { up = 0, right = 1, down = 2, left = 3 };

typedef std::map<std::tuple<int, int>, int> map;

map run_robot(Computer computer, map map) {
    std::deque<code> input = {};

    int rx = 0;
    int ry = 0;
    Direction rd = Direction::up;

    while (true) {
        assert(input.size() == 0);
        input.push_back(map[{rx, ry}]);
        auto [output, halted] = computer.run(input);
        assert(output.size() == 2);
        map[{rx, ry}] = output[0];
        rd = static_cast<Direction>((static_cast<int>(rd) + (output[1] ? 1 : 3)) % 4);
        rx = rx + (rd == Direction::right ? 1 : rd == Direction::left ? -1 : 0);
        ry = ry + (rd == Direction::up ? 1 : rd == Direction::down ? -1 : 0);
        if (halted) {
            break;
        }
    }

    return map;
}

void part1(Program program) {
    auto computer = Computer(program);
    auto map = run_robot(computer, {});
    std::cout << std::format("Part 1: {}\n", map.size());
};

void part2(Program program) {
    auto computer = Computer(program);
    auto map = run_robot(computer, {{{0, 0}, 1}});

    int min_x = 9999, min_y = 9999, max_x = -9999, max_y = -9999;
    for (auto const &[coord, _] : map) {
        auto [x, y] = coord;
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
    }

    std::cout << "Part 2:" << std::endl;
    for (int y = max_y; y >= min_y; y--) {
        for (int x = min_x; x <= max_x; x++) {
            auto it = map.find({x, y});
            std::cout << (it != map.end() && it->second ? "#" : " ");
        }
        std::cout << std::endl;
    }
};

int main() {
    std::ifstream real_input("inputs/day11.txt");
    auto input = &real_input;

    Program program = Program::parse(*input);
    part1(program);
    part2(program);

    return 0;
}
