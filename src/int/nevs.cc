#include "int/nevs.h"

#include <cstdlib>
#include <cstring>

#include "int/intlib.h"
#include "int/memdbg.h"
#include "platform_compat.h"
#include "plib/gnw/debug.h"

namespace fallout {

static constexpr int NEVS_COUNT = 40;

struct Nevs {
    bool used;
    char name[32];
    Program* program;
    int proc;
    int type;
    int hits;
    bool busy;
    NevsCallback* callback;
};

static Nevs* nevs_alloc();
static void nevs_free(Nevs* nevs);
static void nevs_removeprogramreferences(Program* program);
static Nevs* nevs_find(const char* name);

// 0x637728
static Nevs* nevs;

// 0x63772C
static int anyhits;

// 0x47A150
static Nevs* nevs_alloc()
{
    int index;
    Nevs* entry;

    if (nevs == nullptr) {
        debug_printf("nevs_alloc(): nevs_initonce() not called!");
        exit(99);
    }

    for (index = 0; index < NEVS_COUNT; index++) {
        entry = &(nevs[index]);
        if (!entry->used) {
            // NOTE: Uninline.
            nevs_free(entry);
            return entry;
        }
    }

    return nullptr;
}

// 0x47A1A4
static void nevs_free(Nevs* entry)
{
    entry->used = false;
    memset(entry, 0, sizeof(*entry));
}

// 0x47A1BC
void nevs_close()
{
    if (nevs != nullptr) {
        myfree(nevs, __FILE__, __LINE__); // "..\\int\\NEVS.C", 97
        nevs = nullptr;
    }
}

// 0x47A1E4
static void nevs_removeprogramreferences(Program* program)
{
    int index;
    Nevs* entry;

    if (nevs != nullptr) {
        for (index = 0; index < NEVS_COUNT; index++) {
            entry = &(nevs[index]);
            if (entry->used && entry->program == program) {
                // NOTE: Uninline.
                nevs_free(entry);
            }
        }
    }
}

// 0x47A228
void nevs_initonce()
{
    interpretRegisterProgramDeleteCallback(nevs_removeprogramreferences);

    if (nevs == nullptr) {
        nevs = static_cast<Nevs*>(mycalloc(sizeof(Nevs), NEVS_COUNT, __FILE__, __LINE__)); // "..\\int\\NEVS.C", 131
        if (nevs == nullptr) {
            debug_printf("nevs_initonce(): out of memory");
            exit(99);
        }
    }
}

// 0x47A27C
static Nevs* nevs_find(const char* name)
{
    int index;
    Nevs* entry;

    if (nevs == nullptr) {
        debug_printf("nevs_find(): nevs_initonce() not called!");
        exit(99);
    }

    for (index = 0; index < NEVS_COUNT; index++) {
        entry = &(nevs[index]);
        if (entry->used && compat_stricmp(entry->name, name) == 0) {
            return entry;
        }
    }

    return nullptr;
}

// 0x47A2D8
int nevs_addevent(const char* name, Program* program, int proc, int type)
{
    Nevs* entry;

    entry = nevs_find(name);
    if (entry == nullptr) {
        entry = nevs_alloc();
    }

    if (entry == nullptr) {
        return 1;
    }

    entry->used = true;
    strcpy(entry->name, name);
    entry->program = program;
    entry->proc = proc;
    entry->type = type;
    entry->callback = nullptr;

    return 0;
}

// 0x47A338
int nevs_addCevent(const char* name, NevsCallback* callback, int type)
{
    Nevs* entry;

    debug_printf("nevs_addCevent( '%s', %p);\n", name, callback);

    entry = nevs_find(name);
    if (entry == nullptr) {
        entry = nevs_alloc();
    }

    if (entry == nullptr) {
        return 1;
    }

    entry->used = true;
    strcpy(entry->name, name);
    entry->program = nullptr;
    entry->proc = 0;
    entry->type = type;
    entry->callback = nullptr;

    return 0;
}

// 0x47A3AC
int nevs_clearevent(const char* name)
{
    Nevs* entry;

    debug_printf("nevs_clearevent( '%s');\n", name);

    entry = nevs_find(name);
    if (entry != nullptr) {
        // NOTE: Uninline.
        nevs_free(entry);
        return 0;
    }

    return 1;
}

// 0x47A43C
int nevs_signal(const char* name)
{
    Nevs* entry;

    debug_printf("nevs_signal( '%s');\n", name);

    entry = nevs_find(name);
    if (entry == nullptr) {
        return 1;
    }

    debug_printf("nep: %p,  used = %u, prog = %p, proc = %d", entry, entry->used, entry->program, entry->proc);

    if (entry->used
        && ((entry->program != nullptr && entry->proc != 0) || entry->callback != nullptr)
        && !entry->busy) {
        entry->hits++;
        anyhits++;
        return 0;
    }

    return 1;
}

// 0x47A4BC
void nevs_update()
{
    int index;
    Nevs* entry;

    if (anyhits == 0) {
        return;
    }

    debug_printf("nevs_update(): we have anyhits = %u\n", anyhits);

    anyhits = 0;

    for (index = 0; index < NEVS_COUNT; index++) {
        entry = &(nevs[index]);
        if (entry->used
            && ((entry->program != nullptr && entry->proc != 0) || entry->callback != nullptr)
            && !entry->busy) {
            if (entry->hits > 0) {
                entry->busy = true;

                entry->hits -= 1;
                anyhits += entry->hits;

                if (entry->callback == nullptr) {
                    entry->program->executeProc(entry->proc);
                } else {
                    entry->callback(entry->name);
                }

                entry->busy = false;

                if (entry->type == NEVS_TYPE_EVENT) {
                    // NOTE: Uninline.
                    nevs_free(entry);
                }
            }
        }
    }
}

} // namespace fallout
