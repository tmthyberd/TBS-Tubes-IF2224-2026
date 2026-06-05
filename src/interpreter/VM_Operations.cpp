#include "VirtualMachine.hpp"

#include <climits>
#include <cmath>
#include <string>

namespace
{
bool add_overflows(int a, int b)
{
    long long result = static_cast<long long>(a) + static_cast<long long>(b);
    return result > INT_MAX || result < INT_MIN;
}

bool sub_overflows(int a, int b)
{
    long long result = static_cast<long long>(a) - static_cast<long long>(b);
    return result > INT_MAX || result < INT_MIN;
}

bool mul_overflows(int a, int b)
{
    long long result = static_cast<long long>(a) * static_cast<long long>(b);
    return result > INT_MAX || result < INT_MIN;
}

void validate_jump_target(int target, std::size_t code_size)
{
    if (target < 0 || target > static_cast<int>(code_size))
    {
        throw InvalidJumpTargetError("jump target " + std::to_string(target) +
                                     " is outside the instruction range");
    }
}
} // namespace

void VirtualMachine::run()
{
    while (!halted_)
        step();
}

void VirtualMachine::step()
{
    if (halted_)
        return;

    if (ip_ < 0 || ip_ >= static_cast<int>(code_.size()))
    {
        halted_ = true;
        return;
    }

    const Instruction instruction = code_[static_cast<std::size_t>(ip_)];
    ip_ += 1;
    execute(instruction);
}

void VirtualMachine::execute(const Instruction &instruction)
{
    switch (instruction.op)
    {
    case OpCode::LIT: execute_lit(instruction); break;
    case OpCode::LOD: execute_lod(instruction); break;
    case OpCode::STO: execute_sto(instruction); break;
    case OpCode::CAL: execute_cal(instruction); break;
    case OpCode::INT: execute_int(instruction); break;
    case OpCode::JMP: execute_jmp(instruction); break;
    case OpCode::JPC: execute_jpc(instruction); break;
    case OpCode::OPR: execute_opr(instruction); break;
    case OpCode::RET: execute_ret(instruction); break;
    default:
        throw InvalidOperationError("unknown opcode encountered during execution");
    }
}

void VirtualMachine::execute_lit(const Instruction &instruction)
{
    if (instruction.has_literal)
        memory_.push(instruction.literal_value);
    else
        memory_.push(VMValue::integer(instruction.operand));
}

void VirtualMachine::execute_lod(const Instruction &instruction)
{
    memory_.push(memory_.load(instruction.level, instruction.operand));
}

void VirtualMachine::execute_sto(const Instruction &instruction)
{
    VMValue value = memory_.pop();
    memory_.store(instruction.level, instruction.operand, value);
}

void VirtualMachine::execute_int(const Instruction &instruction)
{
    int record_size = instruction.operand;
    int to_allocate = record_size;

    if (memory_.current_base() > 0)
    {
        to_allocate = record_size - VM_FRAME_HEADER_SIZE;
        if (to_allocate < 0)
            to_allocate = 0;
    }

    memory_.allocate(to_allocate);
}

void VirtualMachine::execute_jmp(const Instruction &instruction)
{
    validate_jump_target(instruction.operand, code_.size());
    ip_ = instruction.operand;
}

void VirtualMachine::execute_jpc(const Instruction &instruction)
{
    VMValue condition = memory_.pop();
    if (!condition.is_truthy())
    {
        validate_jump_target(instruction.operand, code_.size());
        ip_ = instruction.operand;
    }
}

void VirtualMachine::execute_cal(const Instruction &instruction)
{
    int caller_base = memory_.current_base();

    int static_link = caller_base;
    for (int i = 0; i < instruction.level; ++i)
        static_link = memory_.load(0, 0).as_int();

    int return_address = ip_;
    memory_.push_frame(static_link, caller_base, return_address, 0);

    validate_jump_target(instruction.operand, code_.size());
    ip_ = instruction.operand;
}

void VirtualMachine::execute_ret(const Instruction &instruction)
{
    (void)instruction;

    if (memory_.current_base() > 0)
    {
        VMFrame frame = memory_.pop_frame();
        ip_ = frame.return_address;
    }
    else
    {
        halted_ = true;
    }
}

namespace
{
VMValue numeric_add(const VMValue &a, const VMValue &b)
{
    if (a.is_string() && b.is_string())
        return VMValue::string(a.as_string() + b.as_string());

    if (a.is_integer() && b.is_integer())
    {
        if (add_overflows(a.as_int(), b.as_int()))
            throw InvalidOperationError("integer addition overflow");
        return VMValue::integer(a.as_int() + b.as_int());
    }
    return VMValue::real(a.as_real() + b.as_real());
}

VMValue numeric_sub(const VMValue &a, const VMValue &b)
{
    if (a.is_integer() && b.is_integer())
    {
        if (sub_overflows(a.as_int(), b.as_int()))
            throw InvalidOperationError("integer subtraction overflow");
        return VMValue::integer(a.as_int() - b.as_int());
    }
    return VMValue::real(a.as_real() - b.as_real());
}

VMValue numeric_mul(const VMValue &a, const VMValue &b)
{
    if (a.is_integer() && b.is_integer())
    {
        if (mul_overflows(a.as_int(), b.as_int()))
            throw InvalidOperationError("integer multiplication overflow");
        return VMValue::integer(a.as_int() * b.as_int());
    }
    return VMValue::real(a.as_real() * b.as_real());
}

VMValue numeric_div(const VMValue &a, const VMValue &b)
{
    if (a.is_integer() && b.is_integer())
    {
        if (b.as_int() == 0)
            throw DivisionByZeroError("integer division by zero");
        return VMValue::integer(a.as_int() / b.as_int());
    }
    if (b.as_real() == 0.0)
        throw DivisionByZeroError("real division by zero");
    return VMValue::real(a.as_real() / b.as_real());
}

VMValue numeric_mod(const VMValue &a, const VMValue &b)
{
    if (!a.is_integer() || !b.is_integer())
        throw TypeMismatchError("'mod' requires Integer operands");
    if (b.as_int() == 0)
        throw DivisionByZeroError("modulo by zero");
    return VMValue::integer(a.as_int() % b.as_int());
}

int compare_values(const VMValue &a, const VMValue &b, bool order)
{
    if (a.is_number() && b.is_number())
    {
        double x = a.as_real();
        double y = b.as_real();
        if (x < y) return -1;
        if (x > y) return 1;
        return 0;
    }
    if (a.is_char() && b.is_char())
    {
        if (a.as_char() < b.as_char()) return -1;
        if (a.as_char() > b.as_char()) return 1;
        return 0;
    }
    if (a.is_string() && b.is_string())
        return a.as_string().compare(b.as_string()) < 0 ? -1
             : (a.as_string() == b.as_string() ? 0 : 1);
    if (a.is_boolean() && b.is_boolean())
    {
        if (order)
            throw TypeMismatchError("ordering comparison is not valid for Boolean operands");
        return a.as_bool() == b.as_bool() ? 0 : 1;
    }

    throw TypeMismatchError("incompatible operand types in comparison: " +
                            a.type_name() + " vs " + b.type_name());
}
} // namespace

void VirtualMachine::execute_opr(const Instruction &instruction)
{
    OprCode op = static_cast<OprCode>(instruction.operand);

    switch (op)
    {
    case OprCode::NEG:
    {
        VMValue value = memory_.pop();
        if (value.is_integer())
        {
            if (value.as_int() == INT_MIN)
                throw InvalidOperationError("integer negation overflow");
            memory_.push(VMValue::integer(-value.as_int()));
        }
        else if (value.is_real())
        {
            memory_.push(VMValue::real(-value.as_real()));
        }
        else
        {
            throw TypeMismatchError("unary '-' requires Integer or Real, got " +
                                    value.type_name());
        }
        break;
    }

    case OprCode::ADD:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(numeric_add(a, b));
        break;
    }
    case OprCode::SUB:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(numeric_sub(a, b));
        break;
    }
    case OprCode::MUL:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(numeric_mul(a, b));
        break;
    }
    case OprCode::DIV:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(numeric_div(a, b));
        break;
    }
    case OprCode::MOD:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(numeric_mod(a, b));
        break;
    }

    case OprCode::EQL:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, false) == 0));
        break;
    }
    case OprCode::NEQ:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, false) != 0));
        break;
    }
    case OprCode::LSS:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, true) < 0));
        break;
    }
    case OprCode::LEQ:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, true) <= 0));
        break;
    }
    case OprCode::GTR:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, true) > 0));
        break;
    }
    case OprCode::GEQ:
    {
        VMValue b = memory_.pop();
        VMValue a = memory_.pop();
        memory_.push(VMValue::boolean(compare_values(a, b, true) >= 0));
        break;
    }

    case OprCode::WRT:
    {
        VMValue value = memory_.pop();
        output_ += value.to_output_string();
        break;
    }
    case OprCode::WRTLN:
    {
        if (!memory_.empty_stack())
            output_ += memory_.pop().to_output_string();
        output_ += "\n";
        break;
    }

    default:
        throw InvalidOperationError("unknown OPR operation code " +
                                    std::to_string(instruction.operand));
    }
}
