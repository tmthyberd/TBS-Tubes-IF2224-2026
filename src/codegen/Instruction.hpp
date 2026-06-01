#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "../interpreter/VM_Value.hpp"

enum class OpCode
{
    LIT,
    LOD,
    STO,
    CAL,
    INT,
    JMP,
    JPC,
    OPR,
    RET
};

enum class OprCode
{
    NEG = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    MOD = 6,
    EQL = 7,
    NEQ = 8,
    LSS = 9,
    GEQ = 10,
    GTR = 11,
    LEQ = 12,
    WRT = 13,
    WRTLN = 14
};

inline std::string opcode_to_string(OpCode op)
{
    switch (op)
    {
    case OpCode::LIT: return "LIT";
    case OpCode::LOD: return "LOD";
    case OpCode::STO: return "STO";
    case OpCode::CAL: return "CAL";
    case OpCode::INT: return "INT";
    case OpCode::JMP: return "JMP";
    case OpCode::JPC: return "JPC";
    case OpCode::OPR: return "OPR";
    case OpCode::RET: return "RET";
    }
    return "UNKNOWN";
}

inline std::string oprcode_to_string(OprCode op)
{
    switch (op)
    {
    case OprCode::NEG: return "NEG";
    case OprCode::ADD: return "ADD";
    case OprCode::SUB: return "SUB";
    case OprCode::MUL: return "MUL";
    case OprCode::DIV: return "DIV";
    case OprCode::MOD: return "MOD";
    case OprCode::EQL: return "EQL";
    case OprCode::NEQ: return "NEQ";
    case OprCode::LSS: return "LSS";
    case OprCode::GEQ: return "GEQ";
    case OprCode::GTR: return "GTR";
    case OprCode::LEQ: return "LEQ";
    case OprCode::WRT: return "WRT";
    case OprCode::WRTLN: return "WRTLN";
    }
    return "UNKNOWN";
}

struct Instruction
{
    OpCode op;
    int level;
    int operand;
    VMValue literal_value;
    bool has_literal;

    Instruction()
        : op(OpCode::RET),
          level(0),
          operand(0),
          literal_value(VMValue::none()),
          has_literal(false)
    {
    }

    Instruction(OpCode op_code, int lexical_level, int value)
        : op(op_code),
          level(lexical_level),
          operand(value),
          literal_value(VMValue::none()),
          has_literal(false)
    {
    }

    static Instruction make(OpCode op_code, int lexical_level = 0, int value = 0)
    {
        return Instruction(op_code, lexical_level, value);
    }

    static Instruction literal(const VMValue &value, int lexical_level = 0)
    {
        Instruction instruction(OpCode::LIT, lexical_level, 0);
        instruction.literal_value = value;
        instruction.has_literal = true;
        return instruction;
    }

    static Instruction operation(OprCode operation_code, int lexical_level = 0)
    {
        return Instruction(OpCode::OPR, lexical_level, static_cast<int>(operation_code));
    }

    std::string to_string() const
    {
        std::ostringstream out;
        out << opcode_to_string(op) << " " << level << " ";
        if (op == OpCode::LIT && has_literal)
            out << literal_value.to_output_string();
        else
            out << operand;
        return out.str();
    }
};

inline std::ostream &operator<<(std::ostream &out, const Instruction &instruction)
{
    out << instruction.to_string();
    return out;
}

inline void print_instructions(std::ostream &out, const std::vector<Instruction> &instructions)
{
    for (std::size_t i = 0; i < instructions.size(); ++i)
        out << i << " " << instructions[i] << "\n";
}

#endif
