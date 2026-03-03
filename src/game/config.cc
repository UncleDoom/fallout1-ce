#include "game/config.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "platform_compat.h"
#include "plib/db/db.h"
#include "plib/gnw/memory.h"

namespace fallout {

// 0x426540
bool Config::init()
{
    if (data_.init(INITIAL_CAPACITY, sizeof(ConfigSection)) != 0) {
        return false;
    }

    return true;
}

// 0x42656C
void Config::exit()
{
    for (int sectionIndex = 0; sectionIndex < data_.getSize(); sectionIndex++) {
        assoc_pair& sectionEntry = data_.getEntry(sectionIndex);

        auto* section = static_cast<ConfigSection*>(sectionEntry.data);
        for (int keyValueIndex = 0; keyValueIndex < section->getSize(); keyValueIndex++) {
            assoc_pair& keyValueEntry = section->getEntry(keyValueIndex);

            char** value = static_cast<char**>(keyValueEntry.data);
            mem_free(*value);
            *value = nullptr;
        }

        section->destroy();
    }

    data_.destroy();
}

// Parses command line arguments and adds them into the config.
//
// The expected format of [argv] elements are "[section]key=value", otherwise
// the element is silently ignored.
//
// NOTE: This function trims whitespace in key-value pair, but not in section.
// I don't know if this is intentional or it's bug.
//
// 0x4265D0
bool Config::cmdLineParse(int argc, char** argv)
{
    for (int arg = 0; arg < argc; arg++) {
        char* pch;
        char* string = argv[arg];

        // Find opening bracket.
        pch = strchr(string, '[');
        if (pch == nullptr) {
            continue;
        }

        char* sectionKey = pch + 1;

        // Find closing bracket.
        pch = strchr(sectionKey, ']');
        if (pch == nullptr) {
            continue;
        }

        *pch = '\0';

        char key[260];
        char value[260];
        if (splitLine(pch + 1, key, value)) {
            if (!setString(sectionKey, key, value)) {
                *pch = ']';
                return false;
            }
        }

        *pch = ']';
    }

    return true;
}

// 0x4266E0
bool Config::getString(const char* sectionKey, const char* key, char** valuePtr)
{
    if (sectionKey == nullptr || key == nullptr || valuePtr == nullptr) {
        return false;
    }

    int sectionIndex = data_.search(sectionKey);
    if (sectionIndex == -1) {
        return false;
    }

    assoc_pair& sectionEntry = data_.getEntry(sectionIndex);
    auto* section = static_cast<ConfigSection*>(sectionEntry.data);

    int index = section->search(key);
    if (index == -1) {
        return false;
    }

    assoc_pair& keyValueEntry = section->getEntry(index);
    *valuePtr = *static_cast<char**>(keyValueEntry.data);

    return true;
}

// 0x426728
bool Config::setString(const char* sectionKey, const char* key, const char* value)
{
    if (sectionKey == nullptr || key == nullptr || value == nullptr) {
        return false;
    }

    int sectionIndex = data_.search(sectionKey);
    if (sectionIndex == -1) {
        if (!addSection(sectionKey)) {
            return false;
        }
        sectionIndex = data_.search(sectionKey);
    }

    assoc_pair& sectionEntry = data_.getEntry(sectionIndex);
    auto* section = static_cast<ConfigSection*>(sectionEntry.data);

    int index = section->search(key);
    if (index != -1) {
        assoc_pair& keyValueEntry = section->getEntry(index);

        char** existingValue = static_cast<char**>(keyValueEntry.data);
        mem_free(*existingValue);
        *existingValue = nullptr;

        section->remove(key);
    }

    char* valueCopy = mem_strdup(value);
    if (valueCopy == nullptr) {
        return false;
    }

    if (section->insert(key, &valueCopy) == -1) {
        mem_free(valueCopy);
        return false;
    }

    return true;
}

// 0x4267DC
bool Config::getValue(const char* sectionKey, const char* key, int* valuePtr)
{
    if (valuePtr == nullptr) {
        return false;
    }

    char* stringValue;
    if (!getString(sectionKey, key, &stringValue)) {
        return false;
    }

    *valuePtr = atoi(stringValue);

    return true;
}

// 0x426810
bool Config::getValues(const char* sectionKey, const char* key, int* arr, int count)
{
    if (arr == nullptr || count < 2) {
        return false;
    }

    char* string;
    if (!getString(sectionKey, key, &string)) {
        return false;
    }

    char temp[MAX_LINE_LENGTH];
    string = strncpy(temp, string, MAX_LINE_LENGTH - 1);

    while (1) {
        char* pch = strchr(string, ',');
        if (pch == nullptr) {
            break;
        }

        count--;
        if (count == 0) {
            break;
        }

        *pch = '\0';
        *arr++ = atoi(string);
        string = pch + 1;
    }

    if (count <= 1) {
        *arr = atoi(string);
        return true;
    }

    return false;
}

// 0x4268E0
bool Config::setValue(const char* sectionKey, const char* key, int value)
{
    char stringValue[20];
    compat_itoa(value, stringValue, 10);

    return setString(sectionKey, key, stringValue);
}

// Reads .INI file into config.
//
// 0x426A00
bool Config::load(const char* filePath, bool isDb)
{
    if (filePath == nullptr) {
        return false;
    }

    char string[MAX_LINE_LENGTH];

    if (isDb) {
        DB_FILE* stream = db_fopen(filePath, "rb");
        if (stream != nullptr) {
            while (stream->fgets(string, sizeof(string)) != nullptr) {
                parseLine(string);
            }
            stream->fclose();
        }
    } else {
        FILE* stream = compat_fopen(filePath, "rt");
        if (stream != nullptr) {
            while (fgets(string, sizeof(string), stream) != nullptr) {
                parseLine(string);
            }

            fclose(stream);
        }

        // FIXME: This function returns `true` even if the file was not actually
        // read. I'm pretty sure it's bug.
    }

    return true;
}

// Writes config into .INI file.
//
// 0x426AA4
bool Config::save(const char* filePath, bool isDb)
{
    if (filePath == nullptr) {
        return false;
    }

    if (isDb) {
        DB_FILE* stream = db_fopen(filePath, "wt");
        if (stream == nullptr) {
            return false;
        }

        for (int sectionIndex = 0; sectionIndex < data_.getSize(); sectionIndex++) {
            assoc_pair& sectionEntry = data_.getEntry(sectionIndex);
            stream->fprintf("[%s]\n", sectionEntry.name);

            auto* section = static_cast<ConfigSection*>(sectionEntry.data);
            for (int index = 0; index < section->getSize(); index++) {
                assoc_pair& keyValueEntry = section->getEntry(index);
                stream->fprintf("%s=%s\n", keyValueEntry.name, *static_cast<char**>(keyValueEntry.data));
            }

            stream->fprintf("\n");
        }

        stream->fclose();
    } else {
        FILE* stream = compat_fopen(filePath, "wt");
        if (stream == nullptr) {
            return false;
        }

        for (int sectionIndex = 0; sectionIndex < data_.getSize(); sectionIndex++) {
            assoc_pair& sectionEntry = data_.getEntry(sectionIndex);
            fprintf(stream, "[%s]\n", sectionEntry.name);

            auto* section = static_cast<ConfigSection*>(sectionEntry.data);
            for (int index = 0; index < section->getSize(); index++) {
                assoc_pair& keyValueEntry = section->getEntry(index);
                fprintf(stream, "%s=%s\n", keyValueEntry.name, *static_cast<char**>(keyValueEntry.data));
            }

            fprintf(stream, "\n");
        }

        fclose(stream);
    }

    return true;
}

// Parses a line from .INI file into config.
//
// A line either contains a "[section]" section key or "key=value" pair. In the
// first case section key is not added to config immediately, instead it is
// stored in |section| for later usage. This prevents empty
// sections in the config.
//
// In case of key-value pair it pretty straight forward - it adds key-value
// pair into previously read section key stored in |section|.
//
// Returns `true` when a section was parsed or key-value pair was parsed and
// added to the config, or `false` otherwise.
//
// 0x426C3C
bool Config::parseLine(char* string)
{
    // 0x504C28
    static char section[MAX_LINE_LENGTH] = "unknown";

    char* pch;

    // Find comment marker and truncate the string.
    pch = strchr(string, ';');
    if (pch != nullptr) {
        *pch = '\0';
    }

    // Find opening bracket.
    pch = strchr(string, '[');
    if (pch != nullptr) {
        char* sectionKey = pch + 1;

        // Find closing bracket.
        pch = strchr(sectionKey, ']');
        if (pch != nullptr) {
            *pch = '\0';
            strcpy(section, sectionKey);
            return stripWhiteSpace(section);
        }
    }

    char key[260];
    char value[260];
    if (!splitLine(string, key, value)) {
        return false;
    }

    return setString(section, key, value);
}

// Splits "key=value" pair from [string] and copy appropriate parts into [key]
// and [value] respectively.
//
// Both key and value are trimmed.
//
// 0x426D14
bool Config::splitLine(char* string, char* key, char* value)
{
    if (string == nullptr || key == nullptr || value == nullptr) {
        return false;
    }

    // Find equals character.
    char* pch = strchr(string, '=');
    if (pch == nullptr) {
        return false;
    }

    *pch = '\0';

    strcpy(key, string);
    strcpy(value, pch + 1);

    *pch = '=';

    stripWhiteSpace(key);
    stripWhiteSpace(value);

    return true;
}

// Ensures the config has a section with specified key.
//
// Return `true` if section exists or it was successfully added, or `false`
// otherwise.
//
// 0x426DB8
bool Config::addSection(const char* sectionKey)
{
    if (sectionKey == nullptr) {
        return false;
    }

    if (data_.search(sectionKey) != -1) {
        // Section already exists, no need to do anything.
        return true;
    }

    ConfigSection section;
    if (section.init(INITIAL_CAPACITY, sizeof(char**)) == -1) {
        return false;
    }

    if (data_.insert(sectionKey, &section) == -1) {
        return false;
    }

    return true;
}

// Removes leading and trailing whitespace from the specified string.
//
// 0x426E18
bool Config::stripWhiteSpace(char* string)
{
    if (string == nullptr) {
        return false;
    }

    int length = strlen(string);
    if (length == 0) {
        return true;
    }

    // Starting from the end of the string, loop while it's a whitespace and
    // decrement string length.
    char* pch = string + length - 1;
    while (length != 0 && isspace(*pch)) {
        length--;
        pch--;
    }

    // pch now points to the last non-whitespace character.
    pch[1] = '\0';

    // Starting from the beginning of the string loop while it's a whitespace
    // and decrement string length.
    pch = string;
    while (isspace(*pch)) {
        pch++;
        length--;
    }

    // pch now points for to the first non-whitespace character.
    memmove(string, pch, length + 1);

    return true;
}

// 0x426E98
bool Config::getDouble(const char* sectionKey, const char* key, double* valuePtr)
{
    if (valuePtr == nullptr) {
        return false;
    }

    char* stringValue;
    if (!getString(sectionKey, key, &stringValue)) {
        return false;
    }

    *valuePtr = strtod(stringValue, nullptr);

    return true;
}

// 0x426ECC
bool Config::setDouble(const char* sectionKey, const char* key, double value)
{
    char stringValue[32];
    snprintf(stringValue, sizeof(stringValue), "%.6f", value);

    return setString(sectionKey, key, stringValue);
}

// NOTE: Boolean-typed variant of [getValue].
bool Config::getBool(const char* sectionKey, const char* key, bool* valuePtr)
{
    if (valuePtr == nullptr) {
        return false;
    }

    int integerValue;
    if (!getValue(sectionKey, key, &integerValue)) {
        return false;
    }

    *valuePtr = integerValue != 0;

    return true;
}

// NOTE: Boolean-typed variant of [setValue].
bool Config::setBool(const char* sectionKey, const char* key, bool value)
{
    return setValue(sectionKey, key, value ? 1 : 0);
}

} // namespace fallout
