#include "VM_Memory.hpp"

#include <sstream>

namespace
{
std::string int_text(int value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}
}

VMMemory::VMMemory(std::size_t max_stack_depth, std::size_t max_frame_depth)
    : current_base_(0),
      max_stack_depth_(max_stack_depth),
      max_frame_depth_(max_frame_depth)
{
}

void VMMemory::reset()
{
    slots_.clear();
    stack_.clear();
    frames_.clear();
    current_base_ = 0;
}

void VMMemory::allocate(int size)
{
    if (size < 0)
        throw InvalidMemoryAccessError("cannot allocate negative size " + int_text(size));

    slots_.resize(slots_.size() + static_cast<std::size_t>(size), VMValue::none());
}

void VMMemory::push(const VMValue &value)
{
    if (stack_.size() >= max_stack_depth_)
        throw StackOverflowError("evaluation stack exceeded configured limit");
    stack_.push_back(value);
}

VMValue VMMemory::pop()
{
    if (stack_.empty())
        throw StackUnderflowError("cannot pop from an empty evaluation stack");

    VMValue value = stack_.back();
    stack_.pop_back();
    return value;
}

VMValue VMMemory::peek() const
{
    if (stack_.empty())
        throw StackUnderflowError("cannot peek an empty evaluation stack");
    return stack_.back();
}

VMValue VMMemory::load(int level, int address) const
{
    return slots_.at(static_cast<std::size_t>(resolve_address(level, address)));
}

void VMMemory::store(int level, int address, const VMValue &value)
{
    int resolved = resolve_address(level, address);
    slots_[static_cast<std::size_t>(resolved)] = value;
}

int VMMemory::push_frame(int static_link, int dynamic_link, int return_address, int local_slots)
{
    if (local_slots < 0)
        throw InvalidMemoryAccessError("cannot create frame with negative local slot count");
    if (frames_.size() >= max_frame_depth_)
        throw StackOverflowError("call stack exceeded configured frame limit");

    VMFrame frame;
    frame.base = static_cast<int>(slots_.size());
    frame.static_link = static_link;
    frame.dynamic_link = dynamic_link;
    frame.return_address = return_address;
    frame.previous_base = current_base_;
    frame.previous_memory_size = static_cast<int>(slots_.size());

    slots_.resize(slots_.size() + VM_FRAME_HEADER_SIZE + static_cast<std::size_t>(local_slots),
                  VMValue::none());
    slots_[static_cast<std::size_t>(frame.base)] = VMValue::integer(static_link);
    slots_[static_cast<std::size_t>(frame.base + 1)] = VMValue::integer(dynamic_link);
    slots_[static_cast<std::size_t>(frame.base + 2)] = VMValue::integer(return_address);

    frames_.push_back(frame);
    current_base_ = frame.base;
    return frame.base;
}

VMFrame VMMemory::pop_frame()
{
    if (frames_.empty())
        throw StackUnderflowError("cannot pop frame because no call frame is active");

    VMFrame frame = frames_.back();
    frames_.pop_back();
    slots_.resize(static_cast<std::size_t>(frame.previous_memory_size));
    current_base_ = frame.previous_base;
    return frame;
}

int VMMemory::current_base() const
{
    return current_base_;
}

int VMMemory::size() const
{
    return static_cast<int>(slots_.size());
}

int VMMemory::stack_size() const
{
    return static_cast<int>(stack_.size());
}

bool VMMemory::empty_stack() const
{
    return stack_.empty();
}

const std::vector<VMValue> &VMMemory::slots() const
{
    return slots_;
}

const std::vector<VMValue> &VMMemory::stack() const
{
    return stack_;
}

int VMMemory::resolve_address(int level, int address) const
{
    if (level < 0)
        throw InvalidMemoryAccessError("negative lexical level " + int_text(level));
    if (address < 0)
        throw InvalidMemoryAccessError("negative address " + int_text(address));

    int base = current_base_;
    for (int i = 0; i < level; ++i)
    {
        if (base < 0 || base + 1 >= static_cast<int>(slots_.size()))
            throw InvalidMemoryAccessError("cannot resolve lexical level " + int_text(level));
        base = slots_[static_cast<std::size_t>(base + 1)].as_int();
    }

    int resolved = base + address;
    if (resolved < 0 || resolved >= static_cast<int>(slots_.size()))
    {
        throw InvalidMemoryAccessError("address " + int_text(address) +
                                       " at level " + int_text(level) +
                                       " resolves to slot " + int_text(resolved));
    }
    return resolved;
}
