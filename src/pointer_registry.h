#pragma once


#include <unordered_map>

namespace fallout {

class PointerRegistry {
public:
    static PointerRegistry* shared();

    PointerRegistry();

    int store(void* ptr);
    void* fetch(int ref, bool remove = false);

private:
    std::unordered_map<int, void*> _map;
    int _next;
};

int ptrToInt(void* ptr);
void* intToPtr(int ref, bool remove = false);

} // namespace fallout
