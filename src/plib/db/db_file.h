#pragma once

#include <utility>

#include "plib/db/db.h"

namespace fallout {

// RAII wrapper around DB_FILE* for automatic resource management.
// Closes the file automatically on destruction.
//
// Usage:
//   DbFile file("save.dat", "rb");
//   if (!file) return -1;
//   file.readInt32(&value);
//
class DbFile {
public:
    DbFile() noexcept = default;

    explicit DbFile(const char* filename, const char* mode) noexcept
        : stream_(db_fopen(filename, mode))
    {
    }

    ~DbFile() noexcept
    {
        close();
    }

    // Move-only (non-copyable)
    DbFile(const DbFile&) = delete;
    DbFile& operator=(const DbFile&) = delete;

    DbFile(DbFile&& other) noexcept
        : stream_(std::exchange(other.stream_, nullptr))
    {
    }

    DbFile& operator=(DbFile&& other) noexcept
    {
        if (this != &other) {
            close();
            stream_ = std::exchange(other.stream_, nullptr);
        }
        return *this;
    }

    // Check if the file is open
    explicit operator bool() const noexcept { return stream_ != nullptr; }
    bool isOpen() const noexcept { return stream_ != nullptr; }

    // Access the underlying raw handle (for interop with legacy code)
    DB_FILE* get() const noexcept { return stream_; }

    // Release ownership without closing
    DB_FILE* release() noexcept { return std::exchange(stream_, nullptr); }

    void close() noexcept
    {
        if (stream_ != nullptr) {
            stream_->fclose();
            stream_ = nullptr;
        }
    }

    // --- Read operations ---

    size_t read(void* buf, size_t size, size_t count) { return stream_->fread(buf, size, count); }
    int readByte(unsigned char* c) { return stream_->freadByte(c); }
    int readShort(unsigned short* s) { return stream_->freadShort(s); }
    int readInt(int* i) { return stream_->freadInt(i); }
    int readLong(unsigned long* l) { return stream_->freadLong(l); }
    int readFloat(float* q) { return stream_->freadFloat(q); }

    int readUInt8(unsigned char* v) { return stream_->freadUInt8(v); }
    int readInt8(char* v) { return stream_->freadInt8(v); }
    int readUInt16(unsigned short* v) { return stream_->freadUInt16(v); }
    int readInt16(short* v) { return stream_->freadInt16(v); }
    int readUInt32(unsigned int* v) { return stream_->freadUInt32(v); }
    int readInt32(int* v) { return stream_->freadInt32(v); }
    int readBool(bool* v) { return stream_->freadBool(v); }

    int readUInt8List(unsigned char* arr, int count) { return stream_->freadUInt8List(arr, count); }
    int readInt8List(char* arr, int count) { return stream_->freadInt8List(arr, count); }
    int readInt16List(short* arr, int count) { return stream_->freadInt16List(arr, count); }
    int readInt32List(int* arr, int count) { return stream_->freadInt32List(arr, count); }

    int readByteCount(unsigned char* c, int count) { return stream_->freadByteCount(c, count); }
    int readShortCount(unsigned short* s, int count) { return stream_->freadShortCount(s, count); }
    int readIntCount(int* i, int count) { return stream_->freadIntCount(i, count); }
    int readLongCount(unsigned long* l, int count) { return stream_->freadLongCount(l, count); }
    int readFloatCount(float* q, int count) { return stream_->freadFloatCount(q, count); }

    int getc() { return stream_->fgetc(); }
    int ungetc(int ch) { return stream_->ungetc(ch); }
    char* gets(char* str, size_t size) { return stream_->fgets(str, size); }

    // --- Write operations ---

    size_t write(const void* buf, size_t size, size_t count) { return stream_->fwrite(buf, size, count); }
    int writeByte(unsigned char c) { return stream_->fwriteByte(c); }
    int writeShort(unsigned short s) { return stream_->fwriteShort(s); }
    int writeInt(int i) { return stream_->fwriteInt(i); }
    int writeLong(unsigned long l) { return stream_->fwriteLong(l); }
    int writeFloat(float q) { return stream_->fwriteFloat(q); }

    int writeUInt8(unsigned char v) { return stream_->fwriteUInt8(v); }
    int writeInt8(char v) { return stream_->fwriteInt8(v); }
    int writeUInt16(unsigned short v) { return stream_->fwriteUInt16(v); }
    int writeInt16(short v) { return stream_->fwriteInt16(v); }
    int writeUInt32(unsigned int v) { return stream_->fwriteUInt32(v); }
    int writeInt32(int v) { return stream_->fwriteInt32(v); }
    int writeBool(bool v) { return stream_->fwriteBool(v); }

    int writeUInt8List(unsigned char* arr, int count) { return stream_->fwriteUInt8List(arr, count); }
    int writeInt8List(char* arr, int count) { return stream_->fwriteInt8List(arr, count); }
    int writeInt16List(short* arr, int count) { return stream_->fwriteInt16List(arr, count); }
    int writeInt32List(int* arr, int count) { return stream_->fwriteInt32List(arr, count); }

    int writeByteCount(unsigned char* c, int count) { return stream_->fwriteByteCount(c, count); }
    int writeShortCount(unsigned short* s, int count) { return stream_->fwriteShortCount(s, count); }
    int writeIntCount(int* i, int count) { return stream_->fwriteIntCount(i, count); }
    int writeLongCount(unsigned long* l, int count) { return stream_->fwriteLongCount(l, count); }
    int writeFloatCount(float* q, int count) { return stream_->fwriteFloatCount(q, count); }

    int putc(int ch) { return stream_->fputc(ch); }
    int puts(const char* s) { return stream_->fputs(s); }

    // --- Positioning ---

    int seek(long offset, int origin) { return stream_->fseek(offset, origin); }
    long tell() { return stream_->ftell(); }
    void rewind() { stream_->rewind(); }
    int eof() { return stream_->feof(); }
    long length() { return stream_->filelength(); }

private:
    DB_FILE* stream_ = nullptr;
};

} // namespace fallout
