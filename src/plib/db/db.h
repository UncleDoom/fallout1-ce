#pragma once


#include <cstddef>
#include <cstdio>

#include "plib/assoc/assoc.h"

namespace fallout {

static constexpr int DB_DATABASE_FILE_LIST_CAPACITY = 32;

class DB_DATABASE;

#define INVALID_DATABASE_HANDLE ((DB_DATABASE*)-1)

class DB_FILE {
public:
    // --- Fields ---
    DB_DATABASE* database;
    unsigned int flags;
    int field_8;
    union {
        int field_C;
        FILE* uncompressed_file_stream;
    };
    int field_10;
    int field_14;
    int field_18;
    unsigned char* field_1C;
    unsigned char* field_20;

    // --- Public methods ---
    int fclose();
    int fgetc();
    int fseek(long offset, int origin);
    long ftell();
    void rewind();
    int feof();
    long filelength();
    size_t fread(void* buf, size_t size, size_t count);
    size_t fwrite(const void* buf, size_t size, size_t count);
    char* fgets(char* str, size_t size);
    int ungetc(int ch);
    int fputc(int ch);
    int fputs(const char* s);
    int fprintf(const char* format, ...);
    int freadByte(unsigned char* c);
    int freadShort(unsigned short* s);
    int freadInt(int* i);
    int freadLong(unsigned long* l);
    int freadFloat(float* q);
    int fwriteByte(unsigned char c);
    int fwriteShort(unsigned short s);
    int fwriteInt(int i);
    int fwriteLong(unsigned long l);
    int fwriteFloat(float q);
    int freadByteCount(unsigned char* c, int count);
    int freadShortCount(unsigned short* s, int count);
    int freadIntCount(int* i, int count);
    int freadLongCount(unsigned long* l, int count);
    int freadFloatCount(float* q, int count);
    int fwriteByteCount(unsigned char* c, int count);
    int fwriteShortCount(unsigned short* s, int count);
    int fwriteIntCount(int* i, int count);
    int fwriteLongCount(unsigned long* l, int count);
    int fwriteFloatCount(float* q, int count);
    int freadUInt8(unsigned char* valuePtr);
    int freadInt8(char* valuePtr);
    int freadUInt16(unsigned short* valuePtr);
    int freadInt16(short* valuePtr);
    int freadUInt32(unsigned int* valuePtr);
    int freadInt32(int* valuePtr);
    int freadUInt8List(unsigned char* arr, int count);
    int freadInt8List(char* arr, int count);
    int freadInt16List(short* arr, int count);
    int freadInt32List(int* arr, int count);
    int freadBool(bool* valuePtr);
    int fwriteUInt8(unsigned char value);
    int fwriteInt8(char value);
    int fwriteUInt16(unsigned short value);
    int fwriteInt16(short value);
    int fwriteUInt32(unsigned int value);
    int fwriteInt32(int value);
    int fwriteUInt8List(unsigned char* arr, int count);
    int fwriteInt8List(char* arr, int count);
    int fwriteInt16List(short* arr, int count);
    int fwriteInt32List(int* arr, int count);
    int fwriteBool(bool value);

private:
    int deleteRecord();
    void preloadBuffer();
};

struct dir_entry {
    int flags;
    int offset;
    int length;
    int field_C;
};

using db_read_callback = void();
using db_malloc_func = void*(size_t size);
using db_strdup_func = char*(const char* string);
using db_free_func = void(void* ptr);

class DB_DATABASE {
public:
    // --- Fields ---
    char* datafile;
    FILE* stream;
    char* datafile_path;
    char* patches_path;
    unsigned char should_free_patches_path;
    assoc_array root;
    assoc_array* entries;
    int files_length;
    DB_FILE files[DB_DATABASE_FILE_LIST_CAPACITY];
    unsigned char* hash_table;

    // --- Methods (converted from free functions taking DB_DATABASE*) ---
    int select();
    int close();
    int init_database(const char* datafile, const char* datafile_path);
    void exit_database();
    int init_patches(const char* path);
    void exit_patches();
    int init_hash_table();
    int reset_hash_table();
    int fill_hash_table(const char* path);
    int add_hash_entry(const char* path, int sep);
    int set_hash_value(unsigned int key, unsigned char enabled);
    int get_hash_value(const char* path, int sep, int* value_ptr);
    void exit_hash_table();
};

DB_DATABASE* db_init(const char* datafile, const char* datafile_path, const char* patches_path, int show_cursor);
DB_DATABASE* db_current();
int db_total();
void db_exit();
int db_dir_entry(const char* filePath, dir_entry* de);
int db_read_to_buf(const char* filePath, unsigned char* ptr);
DB_FILE* db_fopen(const char* filename, const char* mode);
int db_get_file_list(const char* filespec, char*** filelist, char*** desclist, int desclen);
void db_free_file_list(char*** file_list, char*** desclist);
void db_register_mem(db_malloc_func* malloc_func, db_strdup_func* strdup_func, db_free_func* free_func);
void db_register_callback(db_read_callback* callback, size_t threshold);
void db_enable_hash_table();
int db_reset_hash_tables();
int db_add_hash_entry(const char* path, int sep);

} // namespace fallout
