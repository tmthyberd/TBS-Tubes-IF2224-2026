#ifndef VM_EXCEPTIONS_HPP
#define VM_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

class VMError : public std::runtime_error
{
public:
    explicit VMError(const std::string &message);
};

class StackOverflowError : public VMError
{
public:
    explicit StackOverflowError(const std::string &message);
};

class StackUnderflowError : public VMError
{
public:
    explicit StackUnderflowError(const std::string &message);
};

class InvalidMemoryAccessError : public VMError
{
public:
    explicit InvalidMemoryAccessError(const std::string &message);
};

class InvalidJumpTargetError : public VMError
{
public:
    explicit InvalidJumpTargetError(const std::string &message);
};

class InvalidOperationError : public VMError
{
public:
    explicit InvalidOperationError(const std::string &message);
};

class DivisionByZeroError : public VMError
{
public:
    explicit DivisionByZeroError(const std::string &message);
};

class TypeMismatchError : public VMError
{
public:
    explicit TypeMismatchError(const std::string &message);
};

#endif
