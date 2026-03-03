#pragma once

#include "plib/assoc/assoc.h"

namespace fallout {

// Representation of .INI section.
//
// It's implemented as a [assoc_array] whose keys are names of .INI file
// key-value pairs, and its values are pointers to strings (char**).
using ConfigSection = assoc_array;

// A representation of .INI file.
//
// Wraps an assoc_array whose keys are section names and values are
// ConfigSection structs (themselves assoc_arrays of key→string pairs).
class Config {
public:
    Config() = default;
    ~Config() = default;

    // Lifecycle
    [[nodiscard]] bool init();
    void exit();

    // Command-line parsing
    [[nodiscard]] bool cmdLineParse(int argc, char** argv);

    // String accessors
    [[nodiscard]] bool getString(const char* sectionKey, const char* key, char** valuePtr);
    [[nodiscard]] bool setString(const char* sectionKey, const char* key, const char* value);

    // Integer accessors
    [[nodiscard]] bool getValue(const char* sectionKey, const char* key, int* valuePtr);
    [[nodiscard]] bool getValues(const char* sectionKey, const char* key, int* arr, int count);
    [[nodiscard]] bool setValue(const char* sectionKey, const char* key, int value);

    // Double accessors
    [[nodiscard]] bool getDouble(const char* sectionKey, const char* key, double* valuePtr);
    [[nodiscard]] bool setDouble(const char* sectionKey, const char* key, double value);

    // Bool accessors
    [[nodiscard]] bool getBool(const char* sectionKey, const char* key, bool* valuePtr);
    [[nodiscard]] bool setBool(const char* sectionKey, const char* key, bool value);

    // File I/O
    [[nodiscard]] bool load(const char* filePath, bool isDb);
    [[nodiscard]] bool save(const char* filePath, bool isDb);

    // Section iteration (used by combatai.cc and similar)
    int getSize() const { return data_.getSize(); }
    assoc_pair& getEntry(int index) { return data_.getEntry(index); }

private:
    static constexpr int INITIAL_CAPACITY = 10;
    static constexpr int MAX_LINE_LENGTH = 256;

    assoc_array data_;

    bool parseLine(char* string);
    static bool splitLine(char* string, char* key, char* value);
    bool addSection(const char* sectionKey);
    static bool stripWhiteSpace(char* string);
};

} // namespace fallout
