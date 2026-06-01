#ifndef VIRTUAL_MACHINE_HPP
#define VIRTUAL_MACHINE_HPP

#include <string>
#include <vector>

#include "../codegen/Instruction.hpp"
#include "VM_Memory.hpp"

class VirtualMachine
{
public:
    explicit VirtualMachine(const std::vector<Instruction> &code)
        : code_(code),
          ip_(0),
          halted_(false)
    {
    }

    void run();
    void step();

    bool is_halted() const
    {
        return halted_;
    }

    int instruction_pointer() const
    {
        return ip_;
    }

    const std::string &output() const
    {
        return output_;
    }

    const std::vector<Instruction> &code() const
    {
        return code_;
    }

    void set_code(const std::vector<Instruction> &code)
    {
        code_ = code;
        ip_ = 0;
        halted_ = false;
        output_.clear();
        memory_.reset();
    }

    VMMemory &memory()
    {
        return memory_;
    }

    const VMMemory &memory() const
    {
        return memory_;
    }

protected:
    std::vector<Instruction> code_;
    VMMemory memory_;
    int ip_;
    bool halted_;
    std::string output_;

    void execute(const Instruction &instruction);
    void execute_lit(const Instruction &instruction);
    void execute_lod(const Instruction &instruction);
    void execute_sto(const Instruction &instruction);
    void execute_cal(const Instruction &instruction);
    void execute_int(const Instruction &instruction);
    void execute_jmp(const Instruction &instruction);
    void execute_jpc(const Instruction &instruction);
    void execute_opr(const Instruction &instruction);
    void execute_ret(const Instruction &instruction);
};

#endif
