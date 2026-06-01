#ifndef VM_VALUE_HPP
#define VM_VALUE_HPP

#include <sstream>
#include <string>

#include "VM_Exceptions.hpp"

enum class VMValueType
{
    NONE,
    INTEGER,
    REAL,
    BOOLEAN,
    CHAR,
    STRING
};

class VMValue
{
public:
    VMValue()
        : type_(VMValueType::NONE),
          int_value_(0),
          real_value_(0.0),
          bool_value_(false),
          char_value_('\0')
    {
    }

    static VMValue none()
    {
        return VMValue();
    }

    static VMValue integer(int value)
    {
        VMValue result;
        result.type_ = VMValueType::INTEGER;
        result.int_value_ = value;
        return result;
    }

    static VMValue real(double value)
    {
        VMValue result;
        result.type_ = VMValueType::REAL;
        result.real_value_ = value;
        return result;
    }

    static VMValue boolean(bool value)
    {
        VMValue result;
        result.type_ = VMValueType::BOOLEAN;
        result.bool_value_ = value;
        return result;
    }

    static VMValue character(char value)
    {
        VMValue result;
        result.type_ = VMValueType::CHAR;
        result.char_value_ = value;
        return result;
    }

    static VMValue string(const std::string &value)
    {
        VMValue result;
        result.type_ = VMValueType::STRING;
        result.string_value_ = value;
        return result;
    }

    VMValueType type() const
    {
        return type_;
    }

    bool is_none() const { return type_ == VMValueType::NONE; }
    bool is_integer() const { return type_ == VMValueType::INTEGER; }
    bool is_real() const { return type_ == VMValueType::REAL; }
    bool is_boolean() const { return type_ == VMValueType::BOOLEAN; }
    bool is_char() const { return type_ == VMValueType::CHAR; }
    bool is_string() const { return type_ == VMValueType::STRING; }
    bool is_number() const { return is_integer() || is_real(); }

    int as_int() const
    {
        if (!is_integer())
            throw TypeMismatchError("expected Integer, got " + type_name());
        return int_value_;
    }

    double as_real() const
    {
        if (is_real())
            return real_value_;
        if (is_integer())
            return static_cast<double>(int_value_);
        throw TypeMismatchError("expected Real-compatible number, got " + type_name());
    }

    bool as_bool() const
    {
        if (!is_boolean())
            throw TypeMismatchError("expected Boolean, got " + type_name());
        return bool_value_;
    }

    char as_char() const
    {
        if (!is_char())
            throw TypeMismatchError("expected Char, got " + type_name());
        return char_value_;
    }

    const std::string &as_string() const
    {
        if (!is_string())
            throw TypeMismatchError("expected String, got " + type_name());
        return string_value_;
    }

    bool is_truthy() const
    {
        if (is_boolean())
            return bool_value_;
        if (is_integer())
            return int_value_ != 0;
        if (is_real())
            return real_value_ != 0.0;
        throw TypeMismatchError("expected Boolean-compatible value, got " + type_name());
    }

    std::string type_name() const
    {
        switch (type_)
        {
        case VMValueType::NONE: return "None";
        case VMValueType::INTEGER: return "Integer";
        case VMValueType::REAL: return "Real";
        case VMValueType::BOOLEAN: return "Boolean";
        case VMValueType::CHAR: return "Char";
        case VMValueType::STRING: return "String";
        }
        return "Unknown";
    }

    std::string to_output_string() const
    {
        std::ostringstream out;
        switch (type_)
        {
        case VMValueType::NONE:
            return "";
        case VMValueType::INTEGER:
            out << int_value_;
            return out.str();
        case VMValueType::REAL:
            out << real_value_;
            return out.str();
        case VMValueType::BOOLEAN:
            return bool_value_ ? "true" : "false";
        case VMValueType::CHAR:
            out << char_value_;
            return out.str();
        case VMValueType::STRING:
            return string_value_;
        }
        return "";
    }

private:
    VMValueType type_;
    int int_value_;
    double real_value_;
    bool bool_value_;
    char char_value_;
    std::string string_value_;
};

#endif
