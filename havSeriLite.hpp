/*
havSeriLite.hpp

ABOUT

Havoc's single-file serialization library for C++.

TODO

- Add more error handling code.

REVISION HISTORY

v0.1 (2024-12-14) - First release.

LICENSE

MIT License

Copyright (c) 2024 René Nicolaus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef HAVSERILITE_HPP
#define HAVSERILITE_HPP

// Add -D_FILE_OFFSET_BITS=64 to CFLAGS for large file support

#ifdef _WIN32
#ifdef _MBCS
#error "_MBCS is defined, but only Unicode is supported"
#endif
#undef _UNICODE
#define _UNICODE
#undef UNICODE
#define UNICODE

#undef NOMINMAX
#define NOMINMAX

#undef STRICT
#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif
#ifdef _MSC_VER
#include <SDKDDKVer.h>
#endif

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

static_assert(sizeof(signed char) == 1, "expected char to be 1 byte");
static_assert(sizeof(unsigned char) == 1, "expected unsigned char to be 1 byte");
static_assert(sizeof(signed char) == 1, "expected int8 to be 1 byte");
static_assert(sizeof(unsigned char) == 1, "expected uint8 to be 1 byte");
static_assert(sizeof(signed short int) == 2, "expected int16 to be 2 bytes");
static_assert(sizeof(unsigned short int) == 2, "expected uint16 to be 2 bytes");
static_assert(sizeof(signed int) == 4, "expected int32 to be 4 bytes");
static_assert(sizeof(unsigned int) == 4, "expected uint32 to be 4 bytes");
static_assert(sizeof(signed long long) == 8, "expected int64 to be 8 bytes");
static_assert(sizeof(unsigned long long) == 8, "expected uint64 to be 8 bytes");

namespace havSeriLite
{
    enum class havSeriLiteValueType : std::uint8_t
    {
        Null,
        Close,
        Boolean,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Double,
        String,
        Array,
        Object
    };

    struct havSeriLiteValue
    {
        struct havSeriLiteStringData
        {
            std::string mValue;
            std::uint32_t GetSize() const { return static_cast<std::uint32_t>(mValue.size()); }
        };

        std::int32_t mDepthLevel = 0;
        havSeriLiteValueType mType;
        std::variant<havSeriLiteStringData, std::uint64_t, std::int64_t, double, std::uint32_t, std::int32_t, std::nullptr_t, bool> mData;

        explicit havSeriLiteValue(std::nullptr_t value, havSeriLiteValueType valueType) : mType(valueType), mData(value) {}

        explicit havSeriLiteValue(bool value) : mType(havSeriLiteValueType::Boolean), mData(value) {}

        explicit havSeriLiteValue(std::int32_t value) : mType(havSeriLiteValueType::Int32), mData(value) {}
        explicit havSeriLiteValue(std::uint32_t value) : mType(havSeriLiteValueType::UInt32), mData(value) {}

        explicit havSeriLiteValue(std::int64_t value) : mType(havSeriLiteValueType::Int64), mData(value) {}
        explicit havSeriLiteValue(std::uint64_t value) : mType(havSeriLiteValueType::UInt64), mData(value) {}

        explicit havSeriLiteValue(double value) : mType(havSeriLiteValueType::Double), mData(value) {}

        explicit havSeriLiteValue(const std::string& value) : mType(havSeriLiteValueType::String), mData(havSeriLiteStringData { value } ) {}
    };

#ifdef _WIN32
    inline std::wstring ConvertStringToWString(const std::string& value)
    {
        int numOfChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1, nullptr, 0);

        std::wstring wstr(numOfChars, 0);

        numOfChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1, &wstr[0], numOfChars);

        if (numOfChars > 0)
        {
            return wstr;
        }

        return std::wstring();
    }
#endif

    class havSeriLiteWriter
    {
    public:
        explicit havSeriLiteWriter(const std::string& fileName) : mFileStream(nullptr, std::fclose)
        {
            if (WriteFile(fileName) == false)
            {
                throw std::runtime_error("Unable to write file: " + fileName);
            }
        }

        ~havSeriLiteWriter() = default;

        void WriteValue(havSeriLiteValue value)
        {
            std::uint8_t type = static_cast<std::underlying_type_t<havSeriLiteValueType>>(value.mType);
            std::fwrite(&type, sizeof(std::uint8_t), 1, mFileStream.get());

            switch (value.mType)
            {
                case havSeriLiteValueType::Boolean: std::fwrite(&std::get<bool>(value.mData), sizeof(bool), 1, mFileStream.get()); break;
                case havSeriLiteValueType::Int32:   std::fwrite(&std::get<std::int32_t>(value.mData), sizeof(std::int32_t), 1, mFileStream.get()); break;
                case havSeriLiteValueType::UInt32:  std::fwrite(&std::get<std::uint32_t>(value.mData), sizeof(std::uint32_t), 1, mFileStream.get()); break;
                case havSeriLiteValueType::Int64:   std::fwrite(&std::get<std::int64_t>(value.mData), sizeof(std::int64_t), 1, mFileStream.get()); break;
                case havSeriLiteValueType::UInt64:  std::fwrite(&std::get<std::uint64_t>(value.mData), sizeof(std::uint64_t), 1, mFileStream.get()); break;
                case havSeriLiteValueType::Double:  std::fwrite(&std::get<double>(value.mData), sizeof(double), 1, mFileStream.get()); break;
                case havSeriLiteValueType::String:
                    {
                        const havSeriLiteValue::havSeriLiteStringData& stringData = std::get<havSeriLiteValue::havSeriLiteStringData>(value.mData);
                        std::uint32_t stringSize = stringData.GetSize();
                        std::fwrite(&stringSize, sizeof(std::uint32_t), 1, mFileStream.get());
                        std::fwrite(stringData.mValue.data(), stringData.GetSize(), 1, mFileStream.get());
                    }
                    break;
                default: break;
            }
        }

        inline void WriteBool(bool value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteInt32(std::int32_t value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteUInt32(std::uint32_t value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteInt64(std::int64_t value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteUInt64(std::uint64_t value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteDouble(double value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteString(const std::string& value) { WriteValue(havSeriLiteValue { value }); }
        inline void WriteArray() { WriteValue(havSeriLiteValue { nullptr, havSeriLiteValueType::Array }); }
        inline void WriteObject() { WriteValue(havSeriLiteValue { nullptr, havSeriLiteValueType::Object }); }
        // Note: Marks the end of the current object or array in the serialized structure.
        inline void WriteClose() { WriteValue(havSeriLiteValue { nullptr, havSeriLiteValueType::Close }); }
        inline void WriteCloseArray() { WriteClose(); }
        inline void WriteCloseObject() { WriteClose(); }

        bool WriteFile(const std::string& fileName)
        {
#ifdef _WIN32
            std::unique_ptr<std::FILE, decltype(&std::fclose)> fileStream(_wfopen(&ConvertStringToWString(fileName)[0], L"wb"), std::fclose);
#else
            std::unique_ptr<std::FILE, decltype(&std::fclose)> fileStream(std::fopen(fileName.c_str(), "wb"), std::fclose);
#endif

            if (fileStream == nullptr)
            {
                std::cerr << "Unable to write file: " << fileName << std::endl;
                return false;
            }

            mFileStream = std::move(fileStream);

            return true;
        }

        void CloseFile()
        {
            mFileStream.reset(nullptr);
        }

    private:
        std::unique_ptr<std::FILE, decltype(&std::fclose)> mFileStream;
    };

    class havSeriLiteReader
    {
    public:
        explicit havSeriLiteReader(const std::string& fileName) : mSize(0), mIndex(0), mDepthLevel(0)
        {
            if (ReadFile(fileName) == false)
            {
                throw std::runtime_error("Unable to read file: " + fileName);
            }
        }

        ~havSeriLiteReader() = default;

        inline bool ValidityCheck(void* destination, std::uint32_t size)
        {
            if (size == 0 || mIndex + size > mSize)
            {
                return false;
            }

            if (destination != nullptr)
            {
                std::copy(reinterpret_cast<const char*>(&mData[mIndex]), reinterpret_cast<const char*>(&mData[mIndex + size]), static_cast<char*>(destination));
            }

            mIndex += size;

            return true;
        }

        template<typename T>
        inline std::optional<T> TryRead(const havSeriLiteValue& value, havSeriLiteValueType expectedType)
        {
            if (value.mType == expectedType)
            {
                if (auto* ptr = std::get_if<T>(&value.mData))
                {
                    return *ptr;
                }
            }

            return std::nullopt;
        }

        inline std::optional<bool> TryReadBool(const havSeriLiteValue& value) { return TryRead<bool>(value, havSeriLiteValueType::Boolean); }
        inline std::optional<std::int32_t> TryReadInt32(const havSeriLiteValue& value) { return TryRead<std::int32_t>(value, havSeriLiteValueType::Int32); }
        inline std::optional<std::uint32_t> TryReadUInt32(const havSeriLiteValue& value) { return TryRead<std::uint32_t>(value, havSeriLiteValueType::UInt32); }
        inline std::optional<std::int64_t> TryReadInt64(const havSeriLiteValue& value) { return TryRead<std::int64_t>(value, havSeriLiteValueType::Int64); }
        inline std::optional<std::uint64_t> TryReadUInt64(const havSeriLiteValue& value) { return TryRead<std::uint64_t>(value, havSeriLiteValueType::UInt64); }
        inline std::optional<double> TryReadDouble(const havSeriLiteValue& value) { return TryRead<double>(value, havSeriLiteValueType::Double); }
        inline std::optional<std::string> TryReadString(const havSeriLiteValue& value)
        {
            auto result = TryRead<havSeriLiteValue::havSeriLiteStringData>(value, havSeriLiteValueType::String);
            return result ? std::optional<std::string>(result->mValue) : std::nullopt;
        }

        havSeriLiteValue ReadValue()
        {
            havSeriLiteValue result { nullptr, havSeriLiteValueType::Null };

            bool valid = ValidityCheck(&result.mType, sizeof(std::uint8_t));

            switch (result.mType)
            {
                case havSeriLiteValueType::Close:
                    --mDepthLevel;
                    break;

                case havSeriLiteValueType::Boolean:
                    result = havSeriLiteValue { false };
                    valid &= ValidityCheck(&std::get<bool>(result.mData), sizeof(bool));
                    break;

                case havSeriLiteValueType::Int32:
                    result = havSeriLiteValue { std::int32_t { 0 } };
                    valid &= ValidityCheck(&std::get<std::int32_t>(result.mData), sizeof(std::int32_t));
                    break;

                case havSeriLiteValueType::UInt32:
                    result = havSeriLiteValue { std::uint32_t { 0 } };
                    valid &= ValidityCheck(&std::get<std::uint32_t>(result.mData), sizeof(std::uint32_t));
                    break;

                case havSeriLiteValueType::Int64:
                    result = havSeriLiteValue { std::int64_t { 0 } };
                    valid &= ValidityCheck(&std::get<std::int64_t>(result.mData), sizeof(std::int64_t));
                    break;

                case havSeriLiteValueType::UInt64:
                    result = havSeriLiteValue { std::uint64_t { 0 } };
                    valid &= ValidityCheck(&std::get<std::uint64_t>(result.mData), sizeof(std::uint64_t));
                    break;

                case havSeriLiteValueType::Double:
                    result = havSeriLiteValue { double { 0.0 } };
                    valid &= ValidityCheck(&std::get<double>(result.mData), sizeof(double));
                    break;

                case havSeriLiteValueType::String:
                    {
                        result = havSeriLiteValue { std::string { "" } };
                        havSeriLiteValue::havSeriLiteStringData& stringData = std::get<havSeriLiteValue::havSeriLiteStringData>(result.mData);
                        std::uint32_t stringSize = 0;
                        valid &= ValidityCheck(&stringSize, sizeof(std::uint32_t));
                        if (valid == true)
                        {
                            stringData.mValue.resize(stringSize);
                            valid &= ValidityCheck(stringData.mValue.data(), stringSize);
                        }
                    }
                    break;

                case havSeriLiteValueType::Array:
                case havSeriLiteValueType::Object:
                    result.mDepthLevel = ++mDepthLevel;
                    break;

                default:
                    valid &= false;
                    break;
            }

            if (valid == false)
            {
                return havSeriLiteValue { nullptr, havSeriLiteValueType::Null };
            }

            return result;
        }

        inline bool ArrayIterator(const std::int32_t& depthLevel, havSeriLiteValue& value)
        {
            return Iterator(depthLevel, value);
        }

        inline bool ObjectIterator(const std::int32_t& depthLevel, havSeriLiteValue& key, havSeriLiteValue& value)
        {
            return Iterator(depthLevel, value, key);
        }

        bool Iterator(const std::int32_t& depthLevel, havSeriLiteValue& value, std::optional<std::reference_wrapper<havSeriLiteValue>> key = std::nullopt)
        {
            havSeriLiteValue tempValue = havSeriLiteValue { nullptr, havSeriLiteValueType::Close };

            while (tempValue.mType != havSeriLiteValueType::Null && mDepthLevel != depthLevel)
            {
                tempValue = ReadValue();
            }

            if (key.has_value() == true)
            {
                key->get() = ReadValue();

                if (key->get().mType == havSeriLiteValueType::Close)
                {
                    return false;
                }
            }

            value = ReadValue();

            if (key.has_value() == false)
            {
                return value.mType != havSeriLiteValueType::Close;
            }

            return true;
        }

        void OutputValueType(const havSeriLiteValue& value)
        {
            switch (value.mType)
            {
                case havSeriLiteValueType::Boolean: std::cout << (std::get<bool>(value.mData) ? "true" : "false"); break;
                case havSeriLiteValueType::Int32:   std::cout << std::get<std::int32_t>(value.mData); break;
                case havSeriLiteValueType::UInt32:  std::cout << std::get<std::uint32_t>(value.mData); break;
                case havSeriLiteValueType::Int64:   std::cout << std::get<std::int64_t>(value.mData); break;
                case havSeriLiteValueType::UInt64:  std::cout << std::get<std::uint64_t>(value.mData); break;
                case havSeriLiteValueType::Double:  std::cout << std::fixed << std::setprecision(15) << std::get<double>(value.mData); break;
                case havSeriLiteValueType::String:
                    {
                        const auto& stringData = std::get<havSeriLiteValue::havSeriLiteStringData>(value.mData);
                        std::cout << '"';
                        for (const char& character : stringData.mValue)
                        {
                            switch (character)
                            {
                                case '"':  std::cout << "\\\""; break;
                                case '\\': std::cout << "\\\\"; break;
                                case '\b': std::cout << "\\b"; break;
                                case '\f': std::cout << "\\f"; break;
                                case '\n': std::cout << "\\n"; break;
                                case '\r': std::cout << "\\r"; break;
                                case '\t': std::cout << "\\t"; break;
                                case '\v': std::cout << "\\v"; break;
                                default:
                                    if (static_cast<unsigned char>(character) < 0x20)
                                    {
                                        std::cout << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(character));
                                    }
                                    else
                                    {
                                        std::cout << character;
                                    }
                            }
                        }
                        std::cout << '"';
                    }
                    break;
                case havSeriLiteValueType::Null:    std::cerr << "Unsupported or null value type!" << std::endl; break;
                default:                            std::cerr << "Unsupported value type!" << std::endl; break;
            }
        }

        void OutputComplexType(havSeriLiteValue value, std::int32_t depthLevel)
        {
            auto OutputIndent = [](std::int32_t depthLvl) {
                std::cout << std::string(depthLvl * 4, ' ');
            };

            const std::int32_t nextDepth = depthLevel + 1;
            havSeriLiteValue currentKey { nullptr, havSeriLiteValueType::Null };
            havSeriLiteValue currentValue { nullptr, havSeriLiteValueType::Null };
            std::uint32_t elementIndex = 0;

            auto OutputSeparator = [&]() {
                if (++elementIndex > 1)
                {
                    std::cout << "," << std::endl;
                }
            };

            if (value.mType == havSeriLiteValueType::Array)
            {
                std::cout << "[" << std::endl;
                while (ArrayIterator(value.mDepthLevel, currentValue) == true)
                {
                    OutputSeparator();
                    OutputIndent(nextDepth);
                    OutputValue(currentValue, nextDepth);
                }
                std::cout << std::endl;
                OutputIndent(depthLevel);
                std::cout << "]";
            }
            else if (value.mType == havSeriLiteValueType::Object)
            {
                std::cout << "{" << std::endl;
                while (ObjectIterator(value.mDepthLevel, currentKey, currentValue) == true)
                {
                    OutputSeparator();
                    OutputIndent(nextDepth);
                    OutputValue(currentKey, nextDepth);
                    std::cout << ": ";
                    OutputValue(currentValue, nextDepth);
                }
                std::cout << std::endl;
                OutputIndent(depthLevel);
                std::cout << "}";
            }
        }

        // Note: Set "initialState" to "true" to reset the reader's state and start outputting from the beginning.
        void OutputValue(havSeriLiteValue value, std::int32_t depthLevel, bool initialState = false)
        {
            if (initialState == true)
            {
                mIndex = 0;
            }

            if (value.mType == havSeriLiteValueType::Array || value.mType == havSeriLiteValueType::Object)
            {
                OutputComplexType(value, depthLevel);
            }
            else
            {
                OutputValueType(value);
            }
        }

        bool ReadFile(const std::string& fileName)
        {
#ifdef _WIN32
            std::unique_ptr<std::FILE, decltype(&std::fclose)> fileStream(_wfopen(&ConvertStringToWString(fileName)[0], L"rb"), std::fclose);
#else
            std::unique_ptr<std::FILE, decltype(&std::fclose)> fileStream(std::fopen(fileName.c_str(), "rb"), std::fclose);
#endif

            if (fileStream == nullptr)
            {
                std::cerr << "Unable to read file: " << fileName << "!" << std::endl;
                return false;
            }

            mData.clear();

            if (std::fseek(fileStream.get(), 0, SEEK_END) == 0)
            {
                long fileSize = std::ftell(fileStream.get());
                if (fileSize > 0)
                {
                    mData.reserve(static_cast<std::size_t>(fileSize));
                }
                else
                {
                    std::cerr << "Warning: Unable to determine file size, proceeding without reservation!" << std::endl;
                }
                std::fseek(fileStream.get(), 0, SEEK_SET);
            }
            else
            {
                std::cerr << "Error: Failed to seek to end of file, proceeding without reservation!" << std::endl;
            }

            mSize = 0;
            mIndex = 0;
            mDepthLevel = 0;

            char currentChar;

            while (std::fread(&currentChar, sizeof(char), 1, fileStream.get()) > 0)
            {
                mData.push_back(currentChar);
                ++mSize;
            }

            return true;
        }

    private:
        std::vector<char> mData;
        std::uint32_t mSize;
        std::uint32_t mIndex;
        std::int32_t mDepthLevel;
    };
}

#endif
