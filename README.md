# havSeriLite

Havoc's single-file serialization library for C++.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Features

- Intuitive API for seamless integration
- Fast data read and write operations

## Getting Started

This library requires C++17. The library should be cross-platform, but has only been tested under Windows so far.

### Installation

Copy the header file into your project folder and include the file like this:

```cpp
#include "havSeriLite.hpp"
```

### Usage

Here is a sample program demonstrating how to use the library:

```cpp
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#undef UNICODE
#define UNICODE
#include <windows.h>
#endif

#include "havSeriLite.hpp"

struct Person
{
    std::string mFirstName;
    std::string mLastName;
    std::uint32_t mAge;
};

void WritePersonsFile(havSeriLite::havSeriLiteWriter* writer)
{
    writer->WriteArray();
        writer->WriteObject();
            writer->WriteString("FirstName"); writer->WriteString("Joe");
            writer->WriteString("LastName"); writer->WriteString("Smith");
            writer->WriteString("Age"); writer->WriteUInt32(45);
        writer->WriteClose();
        writer->WriteObject();
            writer->WriteString("FirstName"); writer->WriteString("Jane");
            writer->WriteString("LastName"); writer->WriteString("Smith");
            writer->WriteString("Age"); writer->WriteUInt32(40);
        writer->WriteClose();
    writer->WriteClose();
}

Person ReadPerson(havSeriLite::havSeriLiteReader* reader, havSeriLite::havSeriLiteValue value)
{
    Person result { "", "", 0 };

    bool firstNameFound = false;
    bool lastNameFound = false;
    bool ageFound = false;

    havSeriLite::havSeriLiteValue currentKey { nullptr, havSeriLite::havSeriLiteValueType::Null };
    havSeriLite::havSeriLiteValue currentValue { nullptr, havSeriLite::havSeriLiteValueType::Null };

    while (reader->ObjectIterator(value.mDepthLevel, currentKey, currentValue) == true)
    {
        std::string keyValue = *reader->TryReadString(currentKey);

        if (keyValue == "FirstName")
        {
            result.mFirstName = *reader->TryReadString(currentValue);
            firstNameFound = true;
        }
        else if (keyValue == "LastName")
        {
            result.mLastName = *reader->TryReadString(currentValue);
            lastNameFound = true;
        }
        else if (keyValue == "Age")
        {
            result.mAge = *reader->TryReadUInt32(currentValue);
            ageFound = true;
        }
    }

    if (firstNameFound == false || lastNameFound == false || ageFound == false)
    {
        throw std::runtime_error("Incomplete person data!");
    }

    return result;
}

std::size_t ReadPersonsFile(havSeriLite::havSeriLiteReader* reader, havSeriLite::havSeriLiteValue array, std::vector<Person>& persons)
{
    havSeriLite::havSeriLiteValue value { nullptr, havSeriLite::havSeriLiteValueType::Null };

    std::size_t size = 0;

    while (reader->ArrayIterator(array.mDepthLevel, value) == true)
    {
        persons.push_back(ReadPerson(reader, value));

        ++size;
    }

    return size;
}

void WriteComplexFile(havSeriLite::havSeriLiteWriter* writer)
{
    writer->WriteObject();
        writer->WriteString("Example"); writer->WriteObject();
            writer->WriteString("Title"); writer->WriteString("Example");
            writer->WriteString("Test"); writer->WriteObject();
                writer->WriteString("Title"); writer->WriteString("Test");
                writer->WriteString("TestObject"); writer->WriteObject();
                    writer->WriteString("TestEntry"); writer->WriteObject();
                        writer->WriteString("Title"); writer->WriteString("Test");
                        writer->WriteString("Description"); writer->WriteString("T:E:S:T");
                        writer->WriteString("Symbol"); writer->WriteString("★");
                        writer->WriteString("TestDef"); writer->WriteObject();
                            writer->WriteString("Description"); writer->WriteString("A \"test\" definition.");
                            writer->WriteString("Double"); writer->WriteDouble(1.234);
                            writer->WriteString("Values"); writer->WriteArray();
                                writer->WriteString("Test");
                                writer->WriteObject();
                                    writer->WriteString("Int32"); writer->WriteInt32(-1);
                                    writer->WriteString("UInt32"); writer->WriteUInt32(1);
                                    writer->WriteString("Int64"); writer->WriteInt64(-1);
                                    writer->WriteString("UInt64"); writer->WriteUInt64(1);
                                    writer->WriteString("Array"); writer->WriteArray();
                                        writer->WriteString("I'm");
                                        writer->WriteString("an");
                                        writer->WriteString("array");
                                        writer->WriteString("!");
                                    writer->WriteCloseArray();
                                writer->WriteCloseObject();
                            writer->WriteCloseArray();
                        writer->WriteCloseObject();
                        writer->WriteString("Bool"); writer->WriteBool(true);
                    writer->WriteCloseObject();
                writer->WriteCloseObject();
            writer->WriteCloseObject();
        writer->WriteCloseObject();
    writer->WriteCloseObject();
}

int main(int argc, char* argv[])
{
    try
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8); // Ensure UTF-8 output on Windows
#endif

        std::cout << "havSeriLite v0.1" << std::endl << std::endl;

        // Write and Read Persons File
        const std::string personsFileName = "persons.bin";
        auto writer = std::make_unique<havSeriLite::havSeriLiteWriter>(personsFileName);
        WritePersonsFile(writer.get());
        writer->CloseFile();

        auto reader = std::make_unique<havSeriLite::havSeriLiteReader>(personsFileName);
        havSeriLite::havSeriLiteValue root = reader->ReadValue();

        std::vector<Person> persons;
        std::size_t totalNumOfPersons = ReadPersonsFile(reader.get(), root, persons);

        if (totalNumOfPersons == 0)
        {
            throw std::runtime_error("No person records were found in the file!");
        }

        std::cout << "Output #1: " << std::endl << std::endl;
        reader->OutputValue(root, 0, true);
        std::cout << std::endl << std::endl;

        // Write and Read Complex File
        const std::string complexFileName = "complex.bin";
        writer = std::make_unique<havSeriLite::havSeriLiteWriter>(complexFileName);
        WriteComplexFile(writer.get());
        writer->CloseFile();

        reader = std::make_unique<havSeriLite::havSeriLiteReader>(complexFileName);
        root = reader->ReadValue();

        std::cout << "Output #2: " << std::endl << std::endl;
        reader->OutputValue(root, 0);
        std::cout << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
```

## Contributing

Feel free to suggest features or report issues. However, please note that pull requests will not be accepted.

## License

Copyright &copy; 2024 Ren&eacute; Nicolaus

This library is released under the [MIT license](/LICENSE).
