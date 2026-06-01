#ifndef VM_MEMORY_HPP
#define VM_MEMORY_HPP

#include <cstddef>
#include <vector>

#include "VM_Exceptions.hpp"
#include "VM_Value.hpp"

static const int VM_FRAME_HEADER_SIZE = 3;

struct VMFrame
{
    int base;
    int static_link;
    int dynamic_link;
    int return_address;
    int previous_base;
    int previous_memory_size;
};

class VMMemory
{
public:
    explicit VMMemory(std::size_t max_stack_depth = 100000,
                      std::size_t max_frame_depth = 1000);

    void reset();
    void allocate(int size);

    void push(const VMValue &value);
    VMValue pop();
    VMValue peek() const;

    VMValue load(int level, int address) const;
    void store(int level, int address, const VMValue &value);

    int push_frame(int static_link, int dynamic_link, int return_address, int local_slots);
    VMFrame pop_frame();
    int current_base() const;

    int size() const;
    int stack_size() const;
    bool empty_stack() const;

    const std::vector<VMValue> &slots() const;
    const std::vector<VMValue> &stack() const;

private:
    std::vector<VMValue> slots_;
    std::vector<VMValue> stack_;
    std::vector<VMFrame> frames_;
    int current_base_;
    std::size_t max_stack_depth_;
    std::size_t max_frame_depth_;

    int resolve_address(int level, int address) const;
};

#endif
