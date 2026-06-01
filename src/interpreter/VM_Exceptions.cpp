#include "VM_Exceptions.hpp"

VMError::VMError(const std::string &message)
    : std::runtime_error("VM Error: " + message)
{
}

StackOverflowError::StackOverflowError(const std::string &message)
    : VMError("Stack overflow: " + message)
{
}

StackUnderflowError::StackUnderflowError(const std::string &message)
    : VMError("Stack underflow: " + message)
{
}

InvalidMemoryAccessError::InvalidMemoryAccessError(const std::string &message)
    : VMError("Invalid memory access: " + message)
{
}

InvalidJumpTargetError::InvalidJumpTargetError(const std::string &message)
    : VMError("Invalid jump target: " + message)
{
}

InvalidOperationError::InvalidOperationError(const std::string &message)
    : VMError("Invalid operation: " + message)
{
}

DivisionByZeroError::DivisionByZeroError(const std::string &message)
    : VMError("Division by zero: " + message)
{
}

TypeMismatchError::TypeMismatchError(const std::string &message)
    : VMError("Type mismatch: " + message)
{
}
