#ifndef _SECS_ITEM_HH
#define _SECS_ITEM_HH

#include <variant>
#include <vector>
#include <memory>
#include <initializer_list>
#include <string>

namespace SECS
{

class Item;
using ItemPtr = std::shared_ptr<Item>;

namespace ValueType
{
using ASCII = char;
using Binary = int8_t;
using Bool = bool;
using I1 = int8_t;
using I2 = int16_t;
using I4 = int32_t;
using I8 = int64_t;
using U1 = uint8_t;
using U2 = uint16_t;
using U4 = uint32_t;
using U8 = uint64_t;
using F4 = float;
using F8 = double;
using Item = ItemPtr;
};

class Item;
#define ValueTypeList ValueType::ASCII, ValueType::Bool, ValueType::U1, ValueType::U2, ValueType::U4, ValueType::U8, ValueType::I1, ValueType::I2, ValueType::I4, ValueType::I8, ValueType::F4, ValueType::F8, ValueType::Item
using Value = std::variant<ValueTypeList>;

enum class Format
{
    List = 0,		        // 00000000
    Binary = 0x20,		    // 00100000
    Bool = 0x24,		    // 00100100
    ASCII = 0x40,		    // 01000000
    JIS8 = 0x44,			// 01000100
    LCS = 0x48,			    // 01001000
    I8 = 0x60,				// 01100000
    I1 = 0x64,				// 01100100
    I2 = 0x68,				// 01101000
    I4 = 0x70,				// 01110000
    F8 = 0x80,				// 10000000
    F4 = 0x90,				// 10010000
    U8 = 0xA0,				// 10100000
    U1 = 0xA4,				// 10100100
    U2 = 0xA8,				// 10101000
    U4 = 0xB0,				// 10110000
    NONE = 0xFF
};

#define SECS_CREATE_ITEM(type) \
        static ItemPtr type(std::initializer_list<ValueType::type> values = {}) \
        {\
            ItemPtr item = std::make_shared<Item>(Format::type); \
            for (auto&& value : values)\
            {\
                item->append(value);\
            }\
            return item;\
        }

#define SECS_GET_VALUE(type) \
    ValueType::type get##type(size_t index = 0)\
    {\
        return getValue<ValueType::type>(index);\
    }


class Item
{
public:
    Item(Format format = Format::NONE) : m_format(format) {}
    ~Item() = default;

    SECS_CREATE_ITEM(ASCII)
    SECS_CREATE_ITEM(Bool)
    SECS_CREATE_ITEM(Binary)
    SECS_CREATE_ITEM(I1)
    SECS_CREATE_ITEM(I2)
    SECS_CREATE_ITEM(I4)
    SECS_CREATE_ITEM(I8)
    SECS_CREATE_ITEM(U1)
    SECS_CREATE_ITEM(U2)
    SECS_CREATE_ITEM(U4)
    SECS_CREATE_ITEM(U8)
    SECS_CREATE_ITEM(F4)
    SECS_CREATE_ITEM(F8)
    static ItemPtr List(std::initializer_list<ValueType::Item> values = {});
    static ItemPtr String(const std::string &str);

    SECS_GET_VALUE(ASCII)
    SECS_GET_VALUE(Binary)
    SECS_GET_VALUE(Bool)
    SECS_GET_VALUE(I1)
    SECS_GET_VALUE(I2)
    SECS_GET_VALUE(I4)
    SECS_GET_VALUE(I8)
    SECS_GET_VALUE(U1)
    SECS_GET_VALUE(U2)
    SECS_GET_VALUE(U4)
    SECS_GET_VALUE(U8)
    SECS_GET_VALUE(F4)
    SECS_GET_VALUE(F8)
    SECS_GET_VALUE(Item)
    std::string getString();

    void append(const Value &value) {
        m_values.emplace_back(value);
    }

    size_t getCount() {
        return m_values.size();
    }

    template<typename Type>
    Type getValue(size_t index)
    {
        static_assert(std::_Is_any_of_v<Type, ValueTypeList>, "Type is not Secs type");
        return std::get<Type>(m_values[index]);
    }

    Format getFormat() const {
        return m_format;
    };

    int formatSize();

    size_t dataLength();

private:
    std::vector<Value> m_values;
    Format m_format;
};

}

#endif