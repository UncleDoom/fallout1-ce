#include "int/intrpret.h"

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int/export.h"
#include "int/intlib.h"
#include "int/memdbg.h"
#include "platform_compat.h"
#include "plib/color/color.h"
#include "plib/db/db.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"

namespace fallout {

// The maximum number of opcodes.
#define OPCODE_MAX_COUNT 342

// Size of internal stack in bytes (per program).
#define STACK_SIZE 0x800

typedef struct ProgramListNode {
    Program* program;
    struct ProgramListNode* next; // next
    struct ProgramListNode* prev; // prev
} ProgramListNode;

static unsigned int defaultTimerFunc();
static char* defaultFilename(char* fileName);
static int outputStr(char* string);
static int checkWait(Program* program);
static const char* findCurrentProc(Program* program);
static opcode_t fetchWord(unsigned char* data, int pos);
static int fetchLong(unsigned char* a1, int a2);
static void storeWord(int value, unsigned char* a2, int a3);
static void storeLong(int value, unsigned char* stack, int pos);
static void pushShortStack(unsigned char* a1, int* a2, int value);
static void pushLongStack(unsigned char* a1, int* a2, int value);
static int popLongStack(unsigned char* a1, int* a2);
static opcode_t popShortStack(unsigned char* a1, int* a2);
static void detachProgram(Program* program);
static void purgeProgram(Program* program);
static opcode_t getOp(Program* program);
static void checkProgramStrings(Program* program);
static void op_noop(Program* program);
static void op_const(Program* program);
static void op_push_base(Program* program);
static void op_pop_base(Program* program);
static void op_pop_to_base(Program* program);
static void op_set_global(Program* program);
static void op_dump(Program* program);
static void op_call_at(Program* program);
static void op_call_condition(Program* program);
static void op_wait(Program* program);
static void op_cancel(Program* program);
static void op_cancelall(Program* program);
static void op_if(Program* program);
static void op_while(Program* program);
static void op_store(Program* program);
static void op_fetch(Program* program);
static void op_not_equal(Program* program);
static void op_equal(Program* program);
static void op_less_equal(Program* program);
static void op_greater_equal(Program* program);
static void op_less(Program* program);
static void op_greater(Program* program);
static void op_add(Program* program);
static void op_sub(Program* program);
static void op_mul(Program* program);
static void op_div(Program* program);
static void op_mod(Program* program);
static void op_and(Program* program);
static void op_or(Program* program);
static void op_not(Program* program);
static void op_negate(Program* program);
static void op_bwnot(Program* program);
static void op_floor(Program* program);
static void op_bwand(Program* program);
static void op_bwor(Program* program);
static void op_bwxor(Program* program);
static void op_swapa(Program* program);
static void op_critical_done(Program* program);
static void op_critical_start(Program* program);
static void op_jmp(Program* program);
static void op_call(Program* program);
static void op_pop_flags(Program* program);
static void op_pop_return(Program* program);
static void op_pop_exit(Program* program);
static void op_pop_flags_return(Program* program);
static void op_pop_flags_exit(Program* program);
static void op_pop_flags_return_val_exit(Program* program);
static void op_pop_flags_return_val_exit_extern(Program* program);
static void op_pop_flags_return_extern(Program* program);
static void op_pop_flags_exit_extern(Program* program);
static void op_pop_flags_return_val_extern(Program* program);
static void op_pop_address(Program* program);
static void op_a_to_d(Program* program);
static void op_d_to_a(Program* program);
static void op_exit_prog(Program* program);
static void op_stop_prog(Program* program);
static void op_fetch_global(Program* program);
static void op_store_global(Program* program);
static void op_swap(Program* program);
static void op_fetch_proc_address(Program* program);
static void op_pop(Program* program);
static void op_dup(Program* program);
static void op_store_external(Program* program);
static void op_fetch_external(Program* program);
static void op_export_proc(Program* program);
static void op_export_var(Program* program);
static void op_exit(Program* program);
static void op_detach(Program* program);
static void op_callstart(Program* program);
static void op_spawn(Program* program);
static Program* op_fork_helper(Program* program);
static void op_fork(Program* program);
static void op_exec(Program* program);
static void op_check_arg_count(Program* program);
static void op_lookup_string_proc(Program* program);
static void setupCallWithReturnVal(Program* program, int address, int a3);
static void setupCall(Program* program, int address, int returnAddress);
static void setupExternalCallWithReturnVal(Program* program1, Program* program2, int address, int a4);
static void setupExternalCall(Program* program1, Program* program2, int address, int a4);
static void doEvents();
static void removeProgList(ProgramListNode* programListNode);
static void insertProgram(Program* program);

// 0x51903C
static int enabled = 1;

// 0x519040
static InterpretTimerFunc* timerFunc = defaultTimerFunc;

// 0x519044
static unsigned int timerTick = 1000;

// 0x519048
static InterpretMangleFunc* filenameFunc = defaultFilename;

// 0x51904C
static InterpretOutputFunc* outputFunc = outputStr;

// 0x519050
static int cpuBurstSize = 10;

// 0x59E230
static OpcodeHandler* opTable[OPCODE_MAX_COUNT];

// 0x59E788
static unsigned int suspendTime;

// 0x59E78C
static Program* currentProgram;

// 0x59E790
static ProgramListNode* head;

// 0x59E794
static int suspendEvents;

// 0x45B400
static unsigned int defaultTimerFunc()
{
    return get_time();
}

// 0x45B408
void interpretSetTimeFunc(InterpretTimerFunc* timerFunc, int timerTick)
{
    timerFunc = timerFunc;
    timerTick = timerTick;
}

// 0x45B414
static char* defaultFilename(char* fileName)
{
    return fileName;
}

// 0x45B418
char* interpretMangleName(char* fileName)
{
    return filenameFunc(fileName);
}

// 0x45B420
static int outputStr(char* string)
{
    return 1;
}

// 0x45B428
static int checkWait(Program* program)
{
    return 1000 * timerFunc() / timerTick <= program->waitEnd;
}

// 0x45B45C
void interpretOutputFunc(InterpretOutputFunc* func)
{
    outputFunc = func;
}

// 0x45B464
int interpretOutput(const char* format, ...)
{
    if (outputFunc == NULL) {
        return 0;
    }

    char string[260];

    va_list args;
    va_start(args, format);
    int rc = vsnprintf(string, sizeof(string), format, args);
    va_end(args);

    debug_printf(string);

    return rc;
}

// 0x45B4C0
static const char* findCurrentProc(Program* program)
{
    int procedureCount = fetchLong(program->procedures, 0);
    unsigned char* ptr = program->procedures + 4;

    int procedureOffset = fetchLong(ptr, 16);
    int identifierOffset = fetchLong(ptr, 0);

    for (int index = 0; index < procedureCount; index++) {
        int nextProcedureOffset = fetchLong(ptr + sizeof(Procedure), 16);
        if (program->instructionPointer >= procedureOffset && program->instructionPointer < nextProcedureOffset) {
            return (const char*)(program->identifiers + identifierOffset);
        }

        ptr += sizeof(Procedure);
        identifierOffset = fetchLong(ptr, 0);
    }

    return "<couldn't find proc>";
}

// 0x45B5E4
void interpretError(const char* format, ...)
{
    char string[260];

    fadeSystemPalette(cmap, cmap, 0);
    mouse_show();

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(string, sizeof(string), format, argptr);
    va_end(argptr);

    debug_printf("\nError during execution: %s\n", string);

    if (currentProgram == NULL) {
        debug_printf("No current script");
    } else {
        debug_printf("Current script: %s, procedure %s", currentProgram->name, findCurrentProc(currentProgram));
    }

    if (currentProgram) {
        longjmp(currentProgram->env, 1);
    }
}

// 0x45B698
static opcode_t fetchWord(unsigned char* data, int pos)
{
    opcode_t value = 0;
    value |= data[pos++] << 8;
    value |= data[pos++];
    return value;
}

// 0x45B6AC
static int fetchLong(unsigned char* data, int pos)
{
    int value = 0;
    value |= data[pos++] << 24;
    value |= data[pos++] << 16;
    value |= data[pos++] << 8;
    value |= data[pos++] & 0xFF;

    return value;
}

// 0x45B6DC
static void storeWord(int value, unsigned char* stack, int pos)
{
    stack[pos++] = (value >> 8) & 0xFF;
    stack[pos] = value & 0xFF;
}

// 0x45B6F0
static void storeLong(int value, unsigned char* stack, int pos)
{
    stack[pos++] = (value >> 24) & 0xFF;
    stack[pos++] = (value >> 16) & 0xFF;
    stack[pos++] = (value >> 8) & 0xFF;
    stack[pos] = value & 0xFF;
}

// 0x45B72C
static void pushShortStack(unsigned char* data, int* pointer, int value)
{
    if (*pointer + 2 >= STACK_SIZE) {
        interpretError("pushShortStack: Stack overflow.");
    }

    storeWord(value, data, *pointer);

    *pointer += 2;
}

// 0x45B774
static void pushLongStack(unsigned char* data, int* pointer, int value)
{
    int v1;

    if (*pointer + 4 >= STACK_SIZE) {
        // FIXME: Should be pushLongStack.
        interpretError("pushShortStack: Stack overflow.");
    }

    v1 = *pointer;
    storeWord(value >> 16, data, v1);
    storeWord(value & 0xFFFF, data, v1 + 2);
    *pointer = v1 + 4;
}

// 0x45B7CC
static int popLongStack(unsigned char* data, int* pointer)
{
    if (*pointer < 4) {
        interpretError("\nStack underflow long.");
    }

    *pointer -= 4;

    return fetchLong(data, *pointer);
}

// 0x45B814
static opcode_t popShortStack(unsigned char* data, int* pointer)
{
    if (*pointer < 2) {
        interpretError("\nStack underflow short.");
    }

    *pointer -= 2;

    // NOTE: uninline
    return fetchWord(data, *pointer);
}

// 0x45B8D8
static void detachProgram(Program* program)
{
    Program* parent = program->parent;
    if (parent != NULL) {
        parent->flags &= ~PROGRAM_FLAG_0x20;
        parent->flags &= ~PROGRAM_FLAG_0x0100;
        if (program == parent->child) {
            parent->child = NULL;
        }
    }
}

// 0x45B904
static void purgeProgram(Program* program)
{
    if (!program->exited) {
        removeProgramReferences(program);
        program->exited = true;
    }
}

// 0x45B924
void Program::freeProgram()
{
    // NOTE: Uninline.
    detachProgram(this);

    Program* curr = child;
    while (curr != NULL) {
        // NOTE: Uninline.
        purgeProgram(curr);

        curr->parent = NULL;

        Program* next = curr->child;
        curr->child = NULL;

        curr = next;
    }

    // NOTE: Uninline.
    purgeProgram(this);

    if (dynamicStrings != NULL) {
        myfree(dynamicStrings, __FILE__, __LINE__); // "..\int\INTRPRET.C", 371
    }

    if (data != NULL) {
        myfree(data, __FILE__, __LINE__); // "..\int\INTRPRET.C", 372
    }

    if (name != NULL) {
        myfree(name, __FILE__, __LINE__); // "..\int\INTRPRET.C", 373
    }

    stackValues.reset();
    returnStackValues.reset();

    myfree(this, __FILE__, __LINE__); // "..\int\INTRPRET.C", 377
}

// 0x45BA44
Program* allocateProgram(const char* path)
{
    DB_FILE* stream = db_fopen(path, "rb");
    if (stream == NULL) {
        char err[260];
        snprintf(err, sizeof(err), "Couldn't open %s for read\n", path);
        interpretError(err);
        return NULL;
    }

    int fileSize = stream->filelength();
    unsigned char* data = (unsigned char*)mymalloc(fileSize, __FILE__, __LINE__); // ..\int\INTRPRET.C, 398

    stream->fread(data, 1, fileSize);
    stream->fclose();

    Program* program = (Program*)mymalloc(sizeof(Program), __FILE__, __LINE__); // ..\int\INTRPRET.C, 402
    memset(program, 0, sizeof(Program));

    program->name = (char*)mymalloc(strlen(path) + 1, __FILE__, __LINE__); // ..\int\INTRPRET.C, 405
    strcpy(program->name, path);

    program->child = NULL;
    program->parent = NULL;
    program->field_78 = -1;
    program->exited = false;
    program->basePointer = -1;
    program->framePointer = -1;
    program->data = data;
    program->procedures = data + 42;
    program->identifiers = sizeof(Procedure) * fetchLong(program->procedures, 0) + program->procedures + 4;
    program->staticStrings = program->identifiers + fetchLong(program->identifiers, 0) + 4;

    program->stackValues = std::make_unique<ProgramStack>();
    program->returnStackValues = std::make_unique<ProgramStack>();

    return program;
}

// 0x45BC08
static opcode_t getOp(Program* program)
{
    int instructionPointer;

    instructionPointer = program->instructionPointer;
    program->instructionPointer = instructionPointer + 2;

    // NOTE: Uninline.
    return fetchWord(program->data, instructionPointer);
}

// 0x45BC2C
char* Program::getString(opcode_t opcode, int offset)
{
    // The order of checks is important, because dynamic string flag is
    // always used with static string flag.

    if ((opcode & RAW_VALUE_TYPE_DYNAMIC_STRING) != 0) {
        return (char*)(dynamicStrings + 4 + offset);
    }

    if ((opcode & RAW_VALUE_TYPE_STATIC_STRING) != 0) {
        return (char*)(staticStrings + 4 + offset);
    }

    return NULL;
}

// 0x45BC58
char* Program::getName(int offset)
{
    return (char*)(identifiers + offset);
}

// 0x45BC64
int Program::addString(char* string)
{
    int v27;
    unsigned char* v20;
    unsigned char* v23;

    if (this == NULL) {
        return 0;
    }

    v27 = strlen(string) + 1;

    // Align memory
    if (v27 & 1) {
        v27++;
    }

    if (dynamicStrings != NULL) {
        // TODO: Needs testing, lots of pointer stuff.
        unsigned char* heap = dynamicStrings + 4;
        while (*(unsigned short*)heap != 0x8000) {
            short v2 = *(short*)heap;
            if (v2 >= 0) {
                if (v2 == v27) {
                    if (strcmp(string, (char*)(heap + 4)) == 0) {
                        return (heap + 4) - (dynamicStrings + 4);
                    }
                }
            } else {
                v2 = -v2;
                if (v2 > v27) {
                    if (v2 - v27 <= 4) {
                        *(short*)heap = v2;
                    } else {
                        *(short*)(heap + v27 + 6) = 0;
                        *(short*)(heap + v27 + 4) = -(v2 - v27 - 4);
                        *(short*)(heap) = v27;
                    }

                    *(short*)(heap + 2) = 0;
                    strcpy((char*)(heap + 4), string);

                    *(heap + v27 + 3) = '\0';
                    return (heap + 4) - (dynamicStrings + 4);
                }
            }
            heap += v2 + 4;
        }
    } else {
        dynamicStrings = (unsigned char*)mymalloc(8, __FILE__, __LINE__); // "..\int\INTRPRET.C", 459
        *(int*)(dynamicStrings) = 0;
        *(unsigned short*)(dynamicStrings + 4) = 0x8000;
        *(short*)(dynamicStrings + 6) = 1;
    }

    dynamicStrings = (unsigned char*)myrealloc(dynamicStrings, *(int*)(dynamicStrings) + 8 + 4 + v27, __FILE__, __LINE__); // "..\int\INTRPRET.C", 466

    v20 = dynamicStrings + *(int*)(dynamicStrings) + 4;
    if ((*(short*)v20 & 0xFFFF) != 0x8000) {
        interpretError("Internal consistancy error, string table mangled");
    }

    *(int*)(dynamicStrings) += v27 + 4;

    *(short*)(v20) = v27;
    *(short*)(v20 + 2) = 0;

    strcpy((char*)(v20 + 4), string);

    v23 = v20 + v27;
    *(v23 + 3) = '\0';
    *(unsigned short*)(v23 + 4) = 0x8000;
    *(short*)(v23 + 6) = 1;

    return v20 + 4 - (dynamicStrings + 4);
}

// 0x45BDB4
static void op_noop(Program* program)
{
}

// 0x45BDB8
static void op_const(Program* program)
{
    int pos = program->instructionPointer;
    program->instructionPointer = pos + 4;

    int value = fetchLong(program->data, pos);

    ProgramValue result;
    result.opcode = (program->flags >> 16) & 0xFFFF;
    result.integerValue = value;
    program->stackPushValue(result);
}

// - Pops value from stack, which is a number of arguments in the procedure.
// - Saves current frame pointer in return stack.
// - Sets frame pointer to the stack pointer minus number of arguments.
//
// 0x45BE00
static void op_push_base(Program* program)
{
    int argumentCount = program->stackPopInteger();
    program->returnStackPushInteger(program->framePointer);
    program->framePointer = program->stackValues->size() - argumentCount;
}

// 0x45BE5C
static void op_pop_base(Program* program)
{
    int data = program->returnStackPopInteger();
    program->framePointer = data;
}

// 0x45BEBC
static void op_pop_to_base(Program* program)
{
    while (program->stackValues->size() != program->framePointer) {
        program->stackPopValue();
    }
}

// 0x45BEEC
static void op_set_global(Program* program)
{
    program->basePointer = program->stackValues->size();
}

// 0x45BEF8
static void op_dump(Program* program)
{
    int data = program->stackPopInteger();

    // NOTE: Original code is slightly different - it goes backwards to -1.
    for (int index = 0; index < data; index++) {
        program->stackPopValue();
    }
}

// 0x45BF78
static void op_call_at(Program* program)
{
    int data[2];

    for (int arg = 0; arg < 2; arg++) {
        data[arg] = program->stackPopInteger();
    }

    unsigned char* procedure_ptr = program->procedures + 4 + sizeof(Procedure) * data[0];

    int delay = 1000 * data[1] + 1000 * timerFunc() / timerTick;
    int flags = fetchLong(procedure_ptr, 4);

    storeLong(delay, procedure_ptr, 8);
    storeLong(flags | PROCEDURE_FLAG_TIMED, procedure_ptr, 4);
}

// 0x45C0DC
static void op_call_condition(Program* program)
{
    int data[2];

    for (int arg = 0; arg < 2; arg++) {
        data[arg] = program->stackPopInteger();
    }

    unsigned char* procedure_ptr = program->procedures + 4 + sizeof(Procedure) * data[0];
    int flags = fetchLong(procedure_ptr, 4);

    storeLong(flags | PROCEDURE_FLAG_CONDITIONAL, procedure_ptr, 4);
    storeLong(data[1], procedure_ptr, 12);
}

// 0x45C210
static void op_wait(Program* program)
{
    int data = program->stackPopInteger();

    program->waitStart = 1000 * timerFunc() / timerTick;
    program->waitEnd = program->waitStart + data;
    program->checkWaitFunc = checkWait;
    program->flags |= PROGRAM_IS_WAITING;
}

// 0x45C294
static void op_cancel(Program* program)
{
    int data = program->stackPopInteger();

    if (data >= fetchLong(program->procedures, 0)) {
        interpretError("Invalid procedure offset given to cancel");
    }

    Procedure* proc = (Procedure*)(program->procedures + 4 + data * sizeof(*proc));
    proc->field_4 = 0;
    proc->field_8 = 0;
    proc->field_C = 0;
}

// 0x45C3B4
static void op_cancelall(Program* program)
{
    int procedureCount = fetchLong(program->procedures, 0);

    for (int index = 0; index < procedureCount; index++) {
        // TODO: Original code uses different approach, check.
        Procedure* proc = (Procedure*)(program->procedures + 4 + index * sizeof(*proc));

        proc->field_4 = 0;
        proc->field_8 = 0;
        proc->field_C = 0;
    }
}

// 0x45C4A0
static void op_if(Program* program)
{
    ProgramValue value = program->stackPopValue();

    if (!value.isEmpty()) {
        program->stackPopValue();
    } else {
        program->instructionPointer = program->stackPopInteger();
    }
}

// 0x45C4F4
static void op_while(Program* program)
{
    ProgramValue value = program->stackPopValue();

    if (value.isEmpty()) {
        program->instructionPointer = program->stackPopInteger();
    }
}

// 0x45C530
static void op_store(Program* program)
{
    int addr = program->stackPopInteger();
    ProgramValue value = program->stackPopValue();
    size_t pos = program->framePointer + addr;

    program->stackValues->at(pos) = value;
}

// 0x45C5D0
static void op_fetch(Program* program)
{
    int addr = program->stackPopInteger();

    ProgramValue value = program->stackValues->at(program->framePointer + addr);
    program->stackPushValue(value);
}

// 0x45C69C
static void op_not_equal(Program* program)
{
    ProgramValue value[2];
    char stringBuffers[2][80];
    char* strings[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%.5f", value[0].floatValue);
            strings[0] = stringBuffers[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%d", value[0].integerValue);
            strings[0] = stringBuffers[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(strings[1], strings[0]) != 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%.5f", value[1].floatValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) != 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue != value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue != (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%d", value[1].integerValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) != 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue != value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue != value[0].integerValue;
            break;
        case VALUE_TYPE_PTR:
            result = (intptr_t)(value[1].integerValue) != (intptr_t)(value[0].pointerValue);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_INT:
            result = (intptr_t)(value[1].pointerValue) != (intptr_t)(value[0].integerValue);
            break;
        case VALUE_TYPE_PTR:
            result = value[1].pointerValue != value[0].pointerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45C9DC
static void op_equal(Program* program)
{
    ProgramValue value[2];
    char stringBuffers[2][80];
    char* strings[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%.5f", value[0].floatValue);
            strings[0] = stringBuffers[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%d", value[0].integerValue);
            strings[0] = stringBuffers[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(strings[1], strings[0]) == 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%.5f", value[1].floatValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) == 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue == value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue == (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%d", value[1].integerValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) == 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue == value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue == value[0].integerValue;
            break;
        case VALUE_TYPE_PTR:
            result = (intptr_t)(value[1].integerValue) == (intptr_t)(value[0].pointerValue);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_INT:
            result = (intptr_t)(value[1].pointerValue) == (intptr_t)(value[0].integerValue);
            break;
        case VALUE_TYPE_PTR:
            result = value[1].pointerValue == value[0].pointerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45CD1C
static void op_less_equal(Program* program)
{
    ProgramValue value[2];
    char stringBuffers[2][80];
    char* strings[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%.5f", value[0].floatValue);
            strings[0] = stringBuffers[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%d", value[0].integerValue);
            strings[0] = stringBuffers[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(strings[1], strings[0]) <= 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%.5f", value[1].floatValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) <= 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue <= value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue <= (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%d", value[1].integerValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) <= 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue <= value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue <= value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    // Nevada folks tend to use "object <= 0" to test objects for nulls.
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_INT:
            result = (intptr_t)value[1].pointerValue <= (intptr_t)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45D05C
static void op_greater_equal(Program* program)
{
    ProgramValue value[2];
    char stringBuffers[2][80];
    char* strings[2];
    int result;

    // NOTE: original code does not use loop
    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%.5f", value[0].floatValue);
            strings[0] = stringBuffers[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%d", value[0].integerValue);
            strings[0] = stringBuffers[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(strings[1], strings[0]) >= 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%.5f", value[1].floatValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) >= 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue >= value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue >= (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%d", value[1].integerValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) >= 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue >= value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue >= value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45D39C
static void op_less(Program* program)
{
    ProgramValue value[2];
    char text[2][80];
    char* str_ptr[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        str_ptr[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            str_ptr[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(text[0], sizeof(text[0]), "%.5f", value[0].floatValue);
            str_ptr[0] = text[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(text[0], sizeof(text[0]), "%d", value[0].integerValue);
            str_ptr[0] = text[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(str_ptr[1], str_ptr[0]) < 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(text[1], sizeof(text[1]), "%.5f", value[1].floatValue);
            str_ptr[1] = text[1];
            str_ptr[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(str_ptr[1], str_ptr[0]) < 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue < value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue < (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(text[1], sizeof(text[1]), "%d", value[1].integerValue);
            str_ptr[1] = text[1];
            str_ptr[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(str_ptr[1], str_ptr[0]) < 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue < value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue < value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45D6DC
static void op_greater(Program* program)
{
    ProgramValue value[2];
    char stringBuffers[2][80];
    char* strings[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            break;
        case VALUE_TYPE_FLOAT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%.5f", value[0].floatValue);
            strings[0] = stringBuffers[0];
            break;
        case VALUE_TYPE_INT:
            snprintf(stringBuffers[0], sizeof(stringBuffers[0]), "%d", value[0].integerValue);
            strings[0] = stringBuffers[0];
            break;
        default:
            assert(false && "Should be unreachable");
        }

        result = strcmp(strings[1], strings[0]) > 0;
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%.5f", value[1].floatValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) > 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = value[1].floatValue > value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].floatValue > (float)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            snprintf(stringBuffers[1], sizeof(stringBuffers[1]), "%d", value[1].integerValue);
            strings[1] = stringBuffers[1];
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            result = strcmp(strings[1], strings[0]) > 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (float)value[1].integerValue > value[0].floatValue;
            break;
        case VALUE_TYPE_INT:
            result = value[1].integerValue > value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    // Sonora folks tend to use "object > 0" to test objects for nulls.
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_INT:
            result = (intptr_t)value[1].pointerValue > (intptr_t)value[0].integerValue;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45DA1C
static void op_add(Program* program)
{
    ProgramValue value[2];
    char* strings[2];
    char* tempString;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        strings[1] = program->getString(value[1].opcode, value[1].integerValue);

        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            tempString = program->getString(value[0].opcode, value[0].integerValue);
            strings[0] = (char*)mymalloc(strlen(tempString) + 1, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1002
            strcpy(strings[0], tempString);
            break;
        case VALUE_TYPE_FLOAT:
            strings[0] = (char*)mymalloc(80, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1011
            snprintf(strings[0], 80, "%.5f", value[0].floatValue);
            break;
        case VALUE_TYPE_INT:
            strings[0] = (char*)mymalloc(80, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1007
            snprintf(strings[0], 80, "%d", value[0].integerValue);
            break;
        case VALUE_TYPE_PTR:
            strings[0] = (char*)mymalloc(80, __FILE__, __LINE__);
            snprintf(strings[0], 80, "%p", value[0].pointerValue);
            break;
        }

        tempString = (char*)mymalloc(strlen(strings[1]) + strlen(strings[0]) + 1, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1015
        strcpy(tempString, strings[1]);
        strcat(tempString, strings[0]);

        program->stackPushString(tempString);

        myfree(strings[0], __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1019
        myfree(tempString, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1020
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            tempString = (char*)mymalloc(strlen(strings[0]) + 80, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1039
            snprintf(tempString, strlen(strings[0]) + 80, "%.5f", value[1].floatValue);
            strcat(tempString, strings[0]);

            program->stackPushString(tempString);

            myfree(tempString, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1044
            break;
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat(value[1].floatValue + value[0].floatValue);
            break;
        case VALUE_TYPE_INT:
            program->stackPushFloat(value[1].floatValue + (float)value[0].integerValue);
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            strings[0] = program->getString(value[0].opcode, value[0].integerValue);
            tempString = (char*)mymalloc(strlen(strings[0]) + 80, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1070
            snprintf(tempString, strlen(strings[0]) + 80, "%d", value[1].integerValue);
            strcat(tempString, strings[0]);

            program->stackPushString(tempString);

            myfree(tempString, __FILE__, __LINE__); // "..\\int\\INTRPRET.C", 1075
            break;
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat((float)value[1].integerValue + value[0].floatValue);
            break;
        case VALUE_TYPE_INT:
            if ((value[0].integerValue <= 0 || (INT_MAX - value[0].integerValue) > value[1].integerValue)
                && (value[0].integerValue >= 0 || (INT_MIN - value[0].integerValue) <= value[1].integerValue)) {
                program->stackPushInteger(value[1].integerValue + value[0].integerValue);
            } else {
                program->stackPushFloat((float)value[1].integerValue + (float)value[0].integerValue);
            }
            break;
        }
        break;
    }
}

// 0x45DFB0
static void op_sub(Program* program)
{
    ProgramValue value[2];

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat(value[1].floatValue - value[0].floatValue);
            break;
        default:
            program->stackPushFloat(value[1].floatValue - (float)value[0].integerValue);
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat(value[1].integerValue - value[0].floatValue);
            break;
        default:
            program->stackPushInteger(value[1].integerValue - value[0].integerValue);
            break;
        }
        break;
    }
}

// 0x45E09C
static void op_mul(Program* program)
{
    ProgramValue value[2];

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat(value[1].floatValue * value[0].floatValue);
            break;
        default:
            program->stackPushFloat(value[1].floatValue * value[0].integerValue);
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            program->stackPushFloat(value[1].integerValue * value[0].floatValue);
            break;
        default:
            program->stackPushInteger(value[0].integerValue * value[1].integerValue);
            break;
        }
        break;
    }
}

// 0x45E188
static void op_div(Program* program)
{
    ProgramValue value[2];
    float divisor;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        if (value[0].opcode == VALUE_TYPE_FLOAT) {
            divisor = value[0].floatValue;
        } else {
            divisor = (float)value[0].integerValue;
        }

        // NOTE: Original code is slightly different, it performs bitwise and
        // with 0x7FFFFFFF in order to determine if it's zero. Probably some
        // kind of compiler optimization.
        if (divisor == 0.0) {
            interpretError("Division (DIV) by zero");
        }

        program->stackPushFloat(value[1].floatValue / divisor);
        break;
    case VALUE_TYPE_INT:
        if (value[0].opcode == VALUE_TYPE_FLOAT) {
            divisor = value[0].floatValue;

            // NOTE: Same as above.
            if (divisor == 0.0) {
                interpretError("Division (DIV) by zero");
            }

            program->stackPushFloat((float)value[1].integerValue / divisor);
        } else {
            if (value[0].integerValue == 0) {
                interpretError("Division (DIV) by zero");
            }

            program->stackPushInteger(value[1].integerValue / value[0].integerValue);
        }
        break;
    }
}

// 0x45E2EC
static void op_mod(Program* program)
{
    ProgramValue value[2];

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    if (value[1].opcode == VALUE_TYPE_FLOAT) {
        interpretError("Trying to MOD a float");
    }

    if (value[1].opcode != VALUE_TYPE_INT) {
        return;
    }

    if (value[0].opcode == VALUE_TYPE_FLOAT) {
        interpretError("Trying to MOD with a float");
    }

    if (value[0].integerValue == 0) {
        interpretError("Division (MOD) by zero");
    }

    program->stackPushInteger(value[1].integerValue % value[0].integerValue);
}

// 0x45E3B8
static void op_and(Program* program)
{
    ProgramValue value[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = 1;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[0].integerValue & 0x7FFFFFFF) != 0;
            break;
        case VALUE_TYPE_INT:
            result = value[0].integerValue != 0;
            break;
        case VALUE_TYPE_PTR:
            result = value[0].pointerValue != nullptr;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = value[1].integerValue != 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].integerValue & 0x7FFFFFFF) && (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].integerValue & 0x7FFFFFFF) && (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].integerValue & 0x7FFFFFFF) && (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = value[1].integerValue != 0;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].integerValue != 0) && (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].integerValue != 0) && (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].integerValue != 0) && (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = value[1].pointerValue != nullptr;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].pointerValue != nullptr) && (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].pointerValue != nullptr) && (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].pointerValue != nullptr) && (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45E5A0
static void op_or(Program* program)
{
    ProgramValue value[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_DYNAMIC_STRING:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
        case VALUE_TYPE_FLOAT:
        case VALUE_TYPE_INT:
        case VALUE_TYPE_PTR:
            result = 1;
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = 1;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].integerValue & 0x7FFFFFFF) || (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].integerValue & 0x7FFFFFFF) || (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].integerValue & 0x7FFFFFFF) || (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = 1;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].integerValue != 0) || (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].integerValue != 0) || (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].integerValue != 0) || (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    case VALUE_TYPE_PTR:
        switch (value[0].opcode) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_DYNAMIC_STRING:
            result = 1;
            break;
        case VALUE_TYPE_FLOAT:
            result = (value[1].pointerValue != nullptr) || (value[0].integerValue & 0x7FFFFFFF);
            break;
        case VALUE_TYPE_INT:
            result = (value[1].pointerValue != nullptr) || (value[0].integerValue != 0);
            break;
        case VALUE_TYPE_PTR:
            result = (value[1].pointerValue != nullptr) || (value[0].pointerValue != nullptr);
            break;
        default:
            assert(false && "Should be unreachable");
        }
        break;
    default:
        assert(false && "Should be unreachable");
    }

    program->stackPushInteger(result);
}

// 0x45E764
static void op_not(Program* program)
{
    ProgramValue value = program->stackPopValue();
    program->stackPushInteger(value.integerValue == 0);
}

// 0x45E7B0
static void op_negate(Program* program)
{
    // SFALL: Fix vanilla negate operator for float values.
    ProgramValue programValue = program->stackPopValue();
    switch (programValue.opcode) {
    case VALUE_TYPE_INT:
        program->stackPushInteger(-programValue.integerValue);
        break;
    case VALUE_TYPE_FLOAT:
        program->stackPushFloat(-programValue.floatValue);
        break;
    default:
        interpretError("Invalid arg given to NEG");
    }
}

// 0x45E7F0
static void op_bwnot(Program* program)
{
    int value = program->stackPopInteger();
    program->stackPushInteger(~value);
}

// 0x45E830
static void op_floor(Program* program)
{
    ProgramValue value = program->stackPopValue();

    if (value.opcode == VALUE_TYPE_STRING) {
        interpretError("Invalid arg given to floor()");
    } else if (value.opcode == VALUE_TYPE_FLOAT) {
        value.opcode = VALUE_TYPE_INT;
        value.integerValue = (int)value.floatValue;
    }

    program->stackPushValue(value);
}

// 0x45E8BC
static void op_bwand(Program* program)
{
    ProgramValue value[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = (int)value[1].floatValue & (int)value[0].floatValue;
            break;
        default:
            result = (int)value[1].floatValue & value[0].integerValue;
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = value[1].integerValue & (int)value[0].floatValue;
            break;
        default:
            result = value[1].integerValue & value[0].integerValue;
            break;
        }
        break;
    default:
        return;
    }

    program->stackPushInteger(result);
}

// 0x45E9A4
static void op_bwor(Program* program)
{
    ProgramValue value[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = (int)value[1].floatValue | (int)value[0].floatValue;
            break;
        default:
            result = (int)value[1].floatValue | value[0].integerValue;
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = value[1].integerValue | (int)value[0].floatValue;
            break;
        default:
            result = value[1].integerValue | value[0].integerValue;
            break;
        }
        break;
    default:
        return;
    }

    program->stackPushInteger(result);
}

// 0x45EA8C
static void op_bwxor(Program* program)
{
    ProgramValue value[2];
    int result;

    for (int arg = 0; arg < 2; arg++) {
        value[arg] = program->stackPopValue();
    }

    switch (value[1].opcode) {
    case VALUE_TYPE_FLOAT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = (int)value[1].floatValue ^ (int)value[0].floatValue;
            break;
        default:
            result = (int)value[1].floatValue ^ value[0].integerValue;
            break;
        }
        break;
    case VALUE_TYPE_INT:
        switch (value[0].opcode) {
        case VALUE_TYPE_FLOAT:
            result = value[1].integerValue ^ (int)value[0].floatValue;
            break;
        default:
            result = value[1].integerValue ^ value[0].integerValue;
            break;
        }
        break;
    default:
        return;
    }

    program->stackPushInteger(result);
}

// 0x45EB74
static void op_swapa(Program* program)
{
    ProgramValue v1 = program->returnStackPopValue();
    ProgramValue v2 = program->returnStackPopValue();
    program->returnStackPushValue(v1);
    program->returnStackPushValue(v2);
}

// 0x45EBF4
static void op_critical_done(Program* program)
{
    program->flags &= ~PROGRAM_FLAG_CRITICAL_SECTION;
}

// 0x45EBFC
static void op_critical_start(Program* program)
{
    program->flags |= PROGRAM_FLAG_CRITICAL_SECTION;
}

// 0x45EC04
static void op_jmp(Program* program)
{
    program->instructionPointer = program->stackPopInteger();
}

// 0x45EC6C
static void op_call(Program* program)
{
    opcode_t type;
    int data;
    opcode_t argumentType;
    opcode_t argumentValue;
    unsigned char* procedurePtr;
    int procedureFlags;
    char* procedureIdentifier;
    Program* externalProgram;
    int externalProcedureAddress;
    int externalProcedureArgumentCount;
    Program tempProgram;
    char err[256];

    data = program->stackPopInteger();

    procedurePtr = program->procedures + 4 + sizeof(Procedure) * data;

    procedureFlags = fetchLong(procedurePtr, 4);
    if ((procedureFlags & PROCEDURE_FLAG_IMPORTED) != 0) {
        // procedureIdentifier = program->getName(fetchLong(procedurePtr, 0));
        // externalProgram = exportFindProcedure(procedureIdentifier, &externalProcedureAddress, &externalProcedureArgumentCount);
        // if (externalProgram == NULL) {
        //     interpretError("External procedure %s not found", procedureIdentifier);
        // }

        // type = interpretPopShort(program);
        // data = interpretPopLong(program);

        // if ((type & VALUE_TYPE_MASK) != VALUE_TYPE_INT || data != externalProcedureArgumentCount) {
        //     snprintf(err, sizeof(err), "Wrong number of arguments to external procedure %s.Expecting %d, got %d.", procedureIdentifier, externalProcedureArgumentCount, data);
        // }

        // rPushLong(externalProgram, program->instructionPointer);
        // rPushShort(externalProgram, VALUE_TYPE_INT);

        // rPushLong(externalProgram, program->flags);
        // rPushShort(externalProgram, VALUE_TYPE_INT);

        // rPushLong(externalProgram, (int)program->checkWaitFunc);
        // rPushShort(externalProgram, VALUE_TYPE_INT);

        // rPushLong(externalProgram, (int)program);
        // rPushShort(externalProgram, VALUE_TYPE_INT);

        // rPushLong(externalProgram, 36);
        // rPushShort(externalProgram, VALUE_TYPE_INT);

        // interpretPushLong(externalProgram, externalProgram->flags);
        // interpretPushShort(externalProgram, VALUE_TYPE_INT);

        // interpretPushLong(externalProgram, (int)externalProgram->checkWaitFunc);
        // interpretPushShort(externalProgram, VALUE_TYPE_INT);

        // interpretPushLong(externalProgram, externalProgram->windowId);
        // interpretPushShort(externalProgram, VALUE_TYPE_INT);

        // externalProgram->windowId = program->windowId;

        // tempProgram.stackPointer = 0;
        // tempProgram.returnStackPointer = 0;

        // while (data-- != 0) {
        //     argumentType = interpretPopShort(program);
        //     argumentValue = interpretPopLong(program);

        //     interpretPushLong(&tempProgram, argumentValue);
        //     interpretPushShort(&tempProgram, argumentType);
        // }

        // while (data++ < externalProcedureArgumentCount) {
        //     argumentType = interpretPopShort(&tempProgram);
        //     argumentValue = interpretPopLong(&tempProgram);

        //     interpretPushLong(externalProgram, argumentValue);
        //     interpretPushShort(externalProgram, argumentType);
        // }

        // interpretPushLong(externalProgram, externalProcedureArgumentCount);
        // interpretPushShort(externalProgram, VALUE_TYPE_INT);

        // program->flags |= PROGRAM_FLAG_0x20;
        // externalProgram->flags = 0;
        // externalProgram->instructionPointer = externalProcedureAddress;

        // if ((procedureFlags & PROCEDURE_FLAG_CRITICAL) != 0 || (program->flags & PROGRAM_FLAG_CRITICAL_SECTION) != 0) {
        //     // NOTE: Uninline.
        //     op_critical_start(externalProgram);
        // }
    } else {
        program->instructionPointer = fetchLong(procedurePtr, 16);
        if ((procedureFlags & PROCEDURE_FLAG_CRITICAL) != 0) {
            // NOTE: Uninline.
            op_critical_start(program);
        }
    }
}

// 0x45F124
static void op_pop_flags(Program* program)
{
    program->windowId = program->stackPopInteger();
    program->checkWaitFunc = (InterpretCheckWaitFunc*)program->stackPopPointer();
    program->flags = program->stackPopInteger() & 0xFFFF;
}

// pop stack 2 -> set program address
// 0x45F17C
static void op_pop_return(Program* program)
{
    program->instructionPointer = program->returnStackPopInteger();
}

// 0x45F1A0
static void op_pop_exit(Program* program)
{
    program->instructionPointer = program->returnStackPopInteger();

    program->flags |= PROGRAM_FLAG_0x40;
}

// 0x45F1CC
static void op_pop_flags_return(Program* program)
{
    op_pop_flags(program);
    op_pop_return(program);
}

// 0x45F1E8
static void op_pop_flags_exit(Program* program)
{
    op_pop_flags(program);
    op_pop_exit(program);
}

// 0x45F20C
static void op_pop_flags_return_val_exit(Program* program)
{
    ProgramValue value = program->stackPopValue();

    op_pop_flags(program);
    op_pop_exit(program);

    program->stackPushValue(value);
}

// 0x45F26C
static void op_pop_flags_return_val_exit_extern(Program* program)
{
    ProgramValue value = program->stackPopValue();

    op_pop_flags(program);

    Program* v1 = (Program*)program->returnStackPopPointer();
    v1->checkWaitFunc = (InterpretCheckWaitFunc*)program->returnStackPopPointer();
    v1->flags = program->returnStackPopInteger();

    op_pop_exit(program);

    program->stackPushValue(value);
}

// 0x45F32C
static void op_pop_flags_return_extern(Program* program)
{

    op_pop_flags(program);

    Program* v1 = (Program*)program->returnStackPopPointer();
    v1->checkWaitFunc = (InterpretCheckWaitFunc*)program->returnStackPopPointer();
    v1->flags = program->returnStackPopInteger();

    op_pop_return(program);
}

// 0x45F398
static void op_pop_flags_exit_extern(Program* program)
{
    op_pop_flags(program);

    Program* v1 = (Program*)program->returnStackPopPointer();
    v1->checkWaitFunc = (InterpretCheckWaitFunc*)program->returnStackPopPointer();
    v1->flags = program->returnStackPopInteger();

    op_pop_exit(program);
}

// pop value from stack 1 and push it to script popped from stack 2
// 0x45F40C
static void op_pop_flags_return_val_extern(Program* program)
{
    ProgramValue value = program->stackPopValue();

    op_pop_flags(program);

    Program* v10 = (Program*)program->returnStackPopPointer();
    v10->checkWaitFunc = (InterpretCheckWaitFunc*)program->returnStackPopPointer();
    v10->flags = program->returnStackPopInteger();
    if ((value.opcode & 0xF7FF) == VALUE_TYPE_STRING) {
        char* string = program->getString(value.opcode, value.integerValue);
        ProgramValue otherValue;
        otherValue.integerValue = v10->addString(string);
        otherValue.opcode = VALUE_TYPE_DYNAMIC_STRING;
        v10->stackPushValue(otherValue);
    } else {
        v10->stackPushValue(value);
    }

    if (v10->flags & 0x80) {
        program->flags &= ~0x80;
    }

    op_pop_return(program);
    op_pop_return(v10);
}

// 0x45F544
static void op_pop_address(Program* program)
{
    program->returnStackPopValue();
}

// 0x45F564
static void op_a_to_d(Program* program)
{
    ProgramValue value = program->returnStackPopValue();
    program->stackPushValue(value);
}

// 0x45F5AC
static void op_d_to_a(Program* program)
{
    ProgramValue value = program->stackPopValue();
    program->returnStackPushValue(value);
}

// 0x45F5F4
static void op_exit_prog(Program* program)
{
    program->flags |= PROGRAM_FLAG_EXITED;
}

// 0x45F5FC
static void op_stop_prog(Program* program)
{
    program->flags |= PROGRAM_FLAG_STOPPED;
}

// 0x45F604
static void op_fetch_global(Program* program)
{
    int addr = program->stackPopInteger();

    ProgramValue value = program->stackValues->at(program->basePointer + addr);
    program->stackPushValue(value);
}

// 0x45F69C
static void op_store_global(Program* program)
{
    int addr = program->stackPopInteger();
    ProgramValue value = program->stackPopValue();

    program->stackValues->at(program->basePointer + addr) = value;
}

// 0x45F73C
static void op_swap(Program* program)
{
    ProgramValue v1 = program->stackPopValue();
    ProgramValue v2 = program->stackPopValue();
    program->stackPushValue(v1);
    program->stackPushValue(v2);
}

// 0x45F7BC
static void op_fetch_proc_address(Program* program)
{
    int procedureIndex = program->stackPopInteger();

    int address = fetchLong(program->procedures + 4 + sizeof(Procedure) * procedureIndex, 16);
    program->stackPushInteger(address);
}

// Pops value from stack and throws it away.
//
// 0x45F874
static void op_pop(Program* program)
{
    program->stackPopValue();
}

// 0x45F894
static void op_dup(Program* program)
{
    ProgramValue value = program->stackPopValue();
    program->stackPushValue(value);
    program->stackPushValue(value);
}

// 0x45F8FC
static void op_store_external(Program* program)
{
    ProgramValue addr = program->stackPopValue();
    ProgramValue value = program->stackPopValue();

    const char* identifier = program->getName(addr.integerValue);

    if (exportStoreVariable(program, identifier, value)) {
        char err[256];
        snprintf(err, sizeof(err), "External variable %s does not exist\n", identifier);
        interpretError(err);
    }
}

// 0x45F990
static void op_fetch_external(Program* program)
{
    ProgramValue addr = program->stackPopValue();

    const char* identifier = program->getName(addr.integerValue);

    ProgramValue value;
    if (exportFetchVariable(program, identifier, value) != 0) {
        char err[256];
        snprintf(err, sizeof(err), "External variable %s does not exist\n", identifier);
        interpretError(err);
    }

    program->stackPushValue(value);
}

// 0x45FA30
static void op_export_proc(Program* program)
{
    int procedureIndex = program->stackPopInteger();
    int argumentCount = program->stackPopInteger();

    unsigned char* proc_ptr = program->procedures + 4 + sizeof(Procedure) * procedureIndex;

    char* procedureName = program->getName(fetchLong(proc_ptr, 0));
    int procedureAddress = fetchLong(proc_ptr, 16);

    if (exportExportProcedure(program, procedureName, procedureAddress, argumentCount) != 0) {
        char err[256];
        snprintf(err, sizeof(err), "Error exporting procedure %s", procedureName);
        interpretError(err);
    }
}

// 0x45FB08
static void op_export_var(Program* program)
{
    ProgramValue addr = program->stackPopValue();

    const char* identifier = program->getName(addr.integerValue);

    if (exportExportVariable(program, identifier)) {
        char err[256];
        snprintf(err, sizeof(err), "External variable %s already exists", identifier);
        interpretError(err);
    }
}

// 0x45FB70
static void op_exit(Program* program)
{
    program->flags |= PROGRAM_FLAG_EXITED;

    Program* parent = program->parent;
    if (parent != NULL) {
        if ((parent->flags & PROGRAM_FLAG_0x0100) != 0) {
            parent->flags &= ~PROGRAM_FLAG_0x0100;
        }
    }

    purgeProgram(program);
}

// 0x45FBBC
static void op_detach(Program* program)
{
    detachProgram(program);
}

// 0x45FBE8
static void op_callstart(Program* program)
{
    if (program->child) {
        interpretError("Error, already have a child process\n");
    }

    program->flags |= PROGRAM_FLAG_0x20;

    char* name = program->stackPopString();

    // NOTE: Uninline.
    program->child = runScript(name);
    if (program->child == NULL) {
        char err[260];
        snprintf(err, sizeof(err), "Error spawning child %s", name);
        interpretError(err);
    }

    program->child->parent = program;
    program->child->windowId = program->windowId;
}

// 0x45FCFC
static void op_spawn(Program* program)
{
    if (program->child) {
        interpretError("Error, already have a child process\n");
    }

    program->flags |= PROGRAM_FLAG_0x0100;

    char* name = program->stackPopString();

    // NOTE: Uninline.
    program->child = runScript(name);
    if (program->child == NULL) {
        char err[260];
        snprintf(err, sizeof(err), "Error spawning child %s", name);
        interpretError(err);
    }

    program->child->parent = program;
    program->child->windowId = program->windowId;

    if ((program->flags & PROGRAM_FLAG_CRITICAL_SECTION) != 0) {
        program->child->flags |= PROGRAM_FLAG_CRITICAL_SECTION;
        program->child->interpret(-1);
    }
}

// 0x45FE30
static Program* op_fork_helper(Program* program)
{
    char* name = program->stackPopString();
    Program* forked = runScript(name);

    if (forked == NULL) {
        char err[256];
        snprintf(err, sizeof(err), "couldn't fork script '%s'", name);
        interpretError(err);
    }

    forked->windowId = program->windowId;

    return forked;
}

// 0x45FE30
static void op_fork(Program* program)
{
    op_fork_helper(program);
}

// 0x45FF04
static void op_exec(Program* program)
{
    Program* parent = program->parent;
    Program* fork = op_fork_helper(program);

    if (parent != NULL) {
        fork->parent = parent;
        parent->child = fork;
    }

    fork->child = NULL;
    program->parent = NULL;

    op_exit(program);
}

// 0x45FF68
static void op_check_arg_count(Program* program)
{
    int expectedArgumentCount = program->stackPopInteger();
    int procedureIndex = program->stackPopInteger();

    int actualArgumentCount = fetchLong(program->procedures + 4 + sizeof(Procedure) * procedureIndex, 20);
    if (actualArgumentCount != expectedArgumentCount) {
        const char* identifier = program->getName(fetchLong(program->procedures + 4 + sizeof(Procedure) * procedureIndex, 0));
        char err[260];
        snprintf(err, sizeof(err), "Wrong number of args to procedure %s\n", identifier);
        interpretError(err);
    }
}

// 0x460048
static void op_lookup_string_proc(Program* program)
{
    const char* procedureNameToLookup = program->stackPopString();
    int procedureCount = fetchLong(program->procedures, 0);

    // Skip procedure count (4 bytes) and main procedure, which cannot be
    // looked up.
    unsigned char* procedurePtr = program->procedures + 4 + sizeof(Procedure);

    // Start with 1 since we've skipped main procedure, which is always at
    // index 0.
    for (int index = 1; index < procedureCount; index++) {
        int offset = fetchLong(procedurePtr, 0);
        const char* procedureName = program->getName(offset);
        if (compat_stricmp(procedureName, procedureNameToLookup) == 0) {
            program->stackPushInteger(index);
            return;
        }

        procedurePtr += sizeof(Procedure);
    }

    char err[260];
    snprintf(err, sizeof(err), "Couldn't find string procedure %s\n", procedureNameToLookup);
    interpretError(err);
}

// 0x460190
void initInterpreter()
{
    enabled = 1;

    // NOTE: The original code has different sorting.
    interpretAddFunc(OPCODE_NOOP, op_noop);
    interpretAddFunc(OPCODE_PUSH, op_const);
    interpretAddFunc(OPCODE_ENTER_CRITICAL_SECTION, op_critical_start);
    interpretAddFunc(OPCODE_LEAVE_CRITICAL_SECTION, op_critical_done);
    interpretAddFunc(OPCODE_JUMP, op_jmp);
    interpretAddFunc(OPCODE_CALL, op_call);
    interpretAddFunc(OPCODE_CALL_AT, op_call_at);
    interpretAddFunc(OPCODE_CALL_WHEN, op_call_condition);
    interpretAddFunc(OPCODE_CALLSTART, op_callstart);
    interpretAddFunc(OPCODE_EXEC, op_exec);
    interpretAddFunc(OPCODE_SPAWN, op_spawn);
    interpretAddFunc(OPCODE_FORK, op_fork);
    interpretAddFunc(OPCODE_A_TO_D, op_a_to_d);
    interpretAddFunc(OPCODE_D_TO_A, op_d_to_a);
    interpretAddFunc(OPCODE_EXIT, op_exit);
    interpretAddFunc(OPCODE_DETACH, op_detach);
    interpretAddFunc(OPCODE_EXIT_PROGRAM, op_exit_prog);
    interpretAddFunc(OPCODE_STOP_PROGRAM, op_stop_prog);
    interpretAddFunc(OPCODE_FETCH_GLOBAL, op_fetch_global);
    interpretAddFunc(OPCODE_STORE_GLOBAL, op_store_global);
    interpretAddFunc(OPCODE_FETCH_EXTERNAL, op_fetch_external);
    interpretAddFunc(OPCODE_STORE_EXTERNAL, op_store_external);
    interpretAddFunc(OPCODE_EXPORT_VARIABLE, op_export_var);
    interpretAddFunc(OPCODE_EXPORT_PROCEDURE, op_export_proc);
    interpretAddFunc(OPCODE_SWAP, op_swap);
    interpretAddFunc(OPCODE_SWAPA, op_swapa);
    interpretAddFunc(OPCODE_POP, op_pop);
    interpretAddFunc(OPCODE_DUP, op_dup);
    interpretAddFunc(OPCODE_POP_RETURN, op_pop_return);
    interpretAddFunc(OPCODE_POP_EXIT, op_pop_exit);
    interpretAddFunc(OPCODE_POP_ADDRESS, op_pop_address);
    interpretAddFunc(OPCODE_POP_FLAGS, op_pop_flags);
    interpretAddFunc(OPCODE_POP_FLAGS_RETURN, op_pop_flags_return);
    interpretAddFunc(OPCODE_POP_FLAGS_EXIT, op_pop_flags_exit);
    interpretAddFunc(OPCODE_POP_FLAGS_RETURN_EXTERN, op_pop_flags_return_extern);
    interpretAddFunc(OPCODE_POP_FLAGS_EXIT_EXTERN, op_pop_flags_exit_extern);
    interpretAddFunc(OPCODE_POP_FLAGS_RETURN_VAL_EXTERN, op_pop_flags_return_val_extern);
    interpretAddFunc(OPCODE_POP_FLAGS_RETURN_VAL_EXIT, op_pop_flags_return_val_exit);
    interpretAddFunc(OPCODE_POP_FLAGS_RETURN_VAL_EXIT_EXTERN, op_pop_flags_return_val_exit_extern);
    interpretAddFunc(OPCODE_CHECK_PROCEDURE_ARGUMENT_COUNT, op_check_arg_count);
    interpretAddFunc(OPCODE_LOOKUP_PROCEDURE_BY_NAME, op_lookup_string_proc);
    interpretAddFunc(OPCODE_POP_BASE, op_pop_base);
    interpretAddFunc(OPCODE_POP_TO_BASE, op_pop_to_base);
    interpretAddFunc(OPCODE_PUSH_BASE, op_push_base);
    interpretAddFunc(OPCODE_SET_GLOBAL, op_set_global);
    interpretAddFunc(OPCODE_FETCH_PROCEDURE_ADDRESS, op_fetch_proc_address);
    interpretAddFunc(OPCODE_DUMP, op_dump);
    interpretAddFunc(OPCODE_IF, op_if);
    interpretAddFunc(OPCODE_WHILE, op_while);
    interpretAddFunc(OPCODE_STORE, op_store);
    interpretAddFunc(OPCODE_FETCH, op_fetch);
    interpretAddFunc(OPCODE_EQUAL, op_equal);
    interpretAddFunc(OPCODE_NOT_EQUAL, op_not_equal);
    interpretAddFunc(OPCODE_LESS_THAN_EQUAL, op_less_equal);
    interpretAddFunc(OPCODE_GREATER_THAN_EQUAL, op_greater_equal);
    interpretAddFunc(OPCODE_LESS_THAN, op_less);
    interpretAddFunc(OPCODE_GREATER_THAN, op_greater);
    interpretAddFunc(OPCODE_ADD, op_add);
    interpretAddFunc(OPCODE_SUB, op_sub);
    interpretAddFunc(OPCODE_MUL, op_mul);
    interpretAddFunc(OPCODE_DIV, op_div);
    interpretAddFunc(OPCODE_MOD, op_mod);
    interpretAddFunc(OPCODE_AND, op_and);
    interpretAddFunc(OPCODE_OR, op_or);
    interpretAddFunc(OPCODE_BITWISE_AND, op_bwand);
    interpretAddFunc(OPCODE_BITWISE_OR, op_bwor);
    interpretAddFunc(OPCODE_BITWISE_XOR, op_bwxor);
    interpretAddFunc(OPCODE_BITWISE_NOT, op_bwnot);
    interpretAddFunc(OPCODE_FLOOR, op_floor);
    interpretAddFunc(OPCODE_NOT, op_not);
    interpretAddFunc(OPCODE_NEGATE, op_negate);
    interpretAddFunc(OPCODE_WAIT, op_wait);
    interpretAddFunc(OPCODE_CANCEL, op_cancel);
    interpretAddFunc(OPCODE_CANCEL_ALL, op_cancelall);
    interpretAddFunc(OPCODE_START_CRITICAL, op_critical_start);
    interpretAddFunc(OPCODE_END_CRITICAL, op_critical_done);

    initIntlib();
    initExport();
}

// 0x46061C
void interpretClose()
{
    exportClose();
    intlibClose();
}

// 0x460628
void interpretEnableInterpreter(int value)
{
    enabled = value;

    if (value) {
        // NOTE: Uninline.
        interpretResumeEvents();
    } else {
        // NOTE: Uninline.
        interpretSuspendEvents();
    }
}

// 0x460658
void Program::interpret(int a2)
{
    // 0x59E798
    static int busy;

    char err[260];

    Program* oldCurrentProgram = currentProgram;

    if (!enabled) {
        return;
    }

    if (busy) {
        return;
    }

    if (exited || (flags & PROGRAM_FLAG_0x20) != 0 || (flags & PROGRAM_FLAG_0x0100) != 0) {
        return;
    }

    if (field_78 == -1) {
        field_78 = 1000 * timerFunc() / timerTick;
    }

    currentProgram = this;

    if (setjmp(env)) {
        currentProgram = oldCurrentProgram;
        flags |= PROGRAM_FLAG_EXITED | PROGRAM_FLAG_0x04;
        return;
    }

    if ((flags & PROGRAM_FLAG_CRITICAL_SECTION) != 0 && a2 < 3) {
        a2 = 3;
    }

    while ((flags & PROGRAM_FLAG_CRITICAL_SECTION) != 0 || --a2 != -1) {
        if ((flags & (PROGRAM_FLAG_EXITED | PROGRAM_FLAG_0x04 | PROGRAM_FLAG_STOPPED | PROGRAM_FLAG_0x20 | PROGRAM_FLAG_0x40 | PROGRAM_FLAG_0x0100)) != 0) {
            break;
        }

        if (exited) {
            break;
        }

        if ((flags & PROGRAM_IS_WAITING) != 0) {
            busy = 1;

            if (checkWaitFunc != NULL) {
                if (!checkWaitFunc(this)) {
                    busy = 0;
                    continue;
                }
            }

            busy = 0;
            checkWaitFunc = NULL;
            flags &= ~PROGRAM_IS_WAITING;
        }

        // NOTE: Uninline.
        opcode_t opcode = getOp(this);

        // TODO: Replace with field_82 and field_80?
        flags &= 0xFFFF;
        flags |= (opcode << 16);

        if (!((opcode >> 8) & 0x80)) {
            snprintf(err, sizeof(err), "Bad opcode %x %c %d.", opcode, opcode, opcode);
            interpretError(err);
        }

        unsigned int opcodeIndex = opcode & 0x3FF;
        OpcodeHandler* handler = opTable[opcodeIndex];
        if (handler == NULL) {
            snprintf(err, sizeof(err), "Undefined opcode %x.", opcode);
            interpretError(err);
        }

        handler(this);
    }

    if ((flags & PROGRAM_FLAG_EXITED) != 0) {
        if (parent != NULL) {
            if (parent->flags & PROGRAM_FLAG_0x20) {
                parent->flags &= ~PROGRAM_FLAG_0x20;
                parent->child = NULL;
                parent = NULL;
            }
        }
    }

    flags &= ~PROGRAM_FLAG_0x40;
    currentProgram = oldCurrentProgram;
}

// Prepares program stacks for executing proc at [address].
//
// 0x460884
static void setupCallWithReturnVal(Program* program, int address, int returnAddress)
{
    // Save current instruction pointer
    program->returnStackPushInteger(program->instructionPointer);

    // Save return address
    program->returnStackPushInteger(returnAddress);

    // Save program flags
    program->stackPushInteger(program->flags & 0xFFFF);

    program->stackPushPointer((void*)program->checkWaitFunc);

    program->stackPushInteger(program->windowId);

    program->flags &= ~0xFFFF;
    program->instructionPointer = address;
}

// 0x46093C
static void setupCall(Program* program, int address, int returnAddress)
{
    setupCallWithReturnVal(program, address, returnAddress);
    program->stackPushInteger(0);
}

// 0x460968
static void setupExternalCallWithReturnVal(Program* program1, Program* program2, int address, int a4)
{
    program2->returnStackPushInteger(program2->instructionPointer);

    program2->returnStackPushInteger(program1->flags & 0xFFFF);

    program2->returnStackPushPointer((void*)program1->checkWaitFunc);

    program2->returnStackPushPointer(program1);

    program2->returnStackPushInteger(a4);

    program2->stackPushInteger(program2->flags & 0xFFFF);

    program2->stackPushPointer((void*)program2->checkWaitFunc);

    program2->stackPushInteger(program2->windowId);

    program2->flags &= ~0xFFFF;
    program2->instructionPointer = address;
    program2->windowId = program1->windowId;

    program1->flags |= PROGRAM_FLAG_0x20;
}

// 0x460A94
static void setupExternalCall(Program* program1, Program* program2, int address, int a4)
{
    setupExternalCallWithReturnVal(program1, program2, address, a4);
    program2->stackPushInteger(0);
}

// 0x461728
void Program::executeProc(int procedureIndex)
{
    unsigned char* procedurePtr;
    char* procedureIdentifier;
    int procedureAddress;
    Program* externalProgram;
    int externalProcedureAddress;
    int externalProcedureArgumentCount;
    int procedureFlags;
    char err[256];

    procedurePtr = procedures + 4 + sizeof(Procedure) * procedureIndex;
    procedureFlags = fetchLong(procedurePtr, 4);
    if ((procedureFlags & PROCEDURE_FLAG_IMPORTED) != 0) {
        procedureIdentifier = getName(fetchLong(procedurePtr, 0));
        externalProgram = exportFindProcedure(procedureIdentifier, &externalProcedureAddress, &externalProcedureArgumentCount);
        if (externalProgram != NULL) {
            if (externalProcedureArgumentCount == 0) {
            } else {
                snprintf(err, sizeof(err), "External procedure cannot take arguments in interrupt context");
                interpretOutput(err);
            }
        } else {
            snprintf(err, sizeof(err), "External procedure %s not found\n", procedureIdentifier);
            interpretOutput(err);
        }

        // NOTE: Uninline.
        setupExternalCall(this, externalProgram, externalProcedureAddress, 28);

        procedurePtr = externalProgram->procedures + 4 + sizeof(Procedure) * procedureIndex;
        procedureFlags = fetchLong(procedurePtr, 4);

        if ((procedureFlags & PROCEDURE_FLAG_CRITICAL) != 0) {
            // NOTE: Uninline.
            op_critical_start(externalProgram);
            externalProgram->interpret(0);
        }
    } else {
        procedureAddress = fetchLong(procedurePtr, 16);

        // NOTE: Uninline.
        setupCall(this, procedureAddress, 20);

        if ((procedureFlags & PROCEDURE_FLAG_CRITICAL) != 0) {
            // NOTE: Uninline.
            op_critical_start(this);
            interpret(0);
        }
    }
}

// Returns index of the procedure with specified name or -1 if no such
// procedure exists.
//
// 0x461938
int Program::findProcedure(const char* name)
{
    int procedureCount = fetchLong(procedures, 0);

    unsigned char* ptr = procedures + 4;
    for (int index = 0; index < procedureCount; index++) {
        int identifierOffset = fetchLong(ptr, offsetof(Procedure, field_0));
        if (compat_stricmp((char*)(identifiers + identifierOffset), name) == 0) {
            return index;
        }

        ptr += sizeof(Procedure);
    }

    return -1;
}

// 0x4619D4
void Program::executeProcedure(int procedureIndex)
{
    unsigned char* procedurePtr;
    char* procedureIdentifier;
    int procedureAddress;
    Program* externalProgram;
    int externalProcedureAddress;
    int externalProcedureArgumentCount;
    int procedureFlags;
    char err[256];
    jmp_buf env;

    procedurePtr = procedures + 4 + sizeof(Procedure) * procedureIndex;
    procedureFlags = fetchLong(procedurePtr, 4);

    if ((procedureFlags & PROCEDURE_FLAG_IMPORTED) != 0) {
        procedureIdentifier = getName(fetchLong(procedurePtr, 0));
        externalProgram = exportFindProcedure(procedureIdentifier, &externalProcedureAddress, &externalProcedureArgumentCount);
        if (externalProgram != NULL) {
            if (externalProcedureArgumentCount == 0) {
                // NOTE: Uninline.
                setupExternalCall(this, externalProgram, externalProcedureAddress, 32);
                memcpy(env, this->env, sizeof(env));
                externalProgram->interpret(-1);
                memcpy(externalProgram->env, env, sizeof(env));
            } else {
                snprintf(err, sizeof(err), "External procedure cannot take arguments in interrupt context");
                interpretOutput(err);
            }
        } else {
            snprintf(err, sizeof(err), "External procedure %s not found\n", procedureIdentifier);
            interpretOutput(err);
        }
    } else {
        procedureAddress = fetchLong(procedurePtr, 16);

        // NOTE: Uninline.
        setupCall(this, procedureAddress, 24);
        memcpy(env, this->env, sizeof(env));
        interpret(-1);
        memcpy(this->env, env, sizeof(env));
    }
}

// 0x461BE8
static void doEvents()
{
    ProgramListNode* programListNode;
    unsigned int time;
    int procedureCount;
    int procedureIndex;
    unsigned char* procedurePtr;
    int procedureFlags;
    int oldProgramFlags;
    int oldInstructionPointer;
    int data;
    jmp_buf env;

    if (suspendEvents) {
        return;
    }

    programListNode = head;
    time = 1000 * timerFunc() / timerTick;

    while (programListNode != NULL) {
        procedureCount = fetchLong(programListNode->program->procedures, 0);

        procedurePtr = programListNode->program->procedures + 4;
        for (procedureIndex = 0; procedureIndex < procedureCount; procedureIndex++) {
            procedureFlags = fetchLong(procedurePtr, 4);
            if ((procedureFlags & PROCEDURE_FLAG_CONDITIONAL) != 0) {
                memcpy(env, programListNode->program, sizeof(env));
                oldProgramFlags = programListNode->program->flags;
                oldInstructionPointer = programListNode->program->instructionPointer;

                programListNode->program->flags = 0;
                programListNode->program->instructionPointer = fetchLong(procedurePtr, 12);
                programListNode->program->interpret(-1);

                if ((programListNode->program->flags & PROGRAM_FLAG_0x04) == 0) {
                    data = programListNode->program->stackPopInteger();

                    programListNode->program->flags = oldProgramFlags;
                    programListNode->program->instructionPointer = oldInstructionPointer;

                    if (data != 0) {
                        // NOTE: Uninline.
                        storeLong(0, procedurePtr, 4);
                        programListNode->program->executeProc(procedureIndex);
                    }
                }

                memcpy(programListNode->program, env, sizeof(env));
            } else if ((procedureFlags & PROCEDURE_FLAG_TIMED) != 0) {
                if ((unsigned int)fetchLong(procedurePtr, 8) < time) {
                    // NOTE: Uninline.
                    storeLong(0, procedurePtr, 4);
                    programListNode->program->executeProc(procedureIndex);
                }
            }
            procedurePtr += sizeof(Procedure);
        }

        programListNode = programListNode->next;
    }
}

// 0x461E48
static void removeProgList(ProgramListNode* programListNode)
{
    ProgramListNode* tmp;

    tmp = programListNode->next;
    if (tmp != NULL) {
        tmp->prev = programListNode->prev;
    }

    tmp = programListNode->prev;
    if (tmp != NULL) {
        tmp->next = programListNode->next;
    } else {
        head = programListNode->next;
    }

    programListNode->program->freeProgram();
    myfree(programListNode, __FILE__, __LINE__); // "..\int\INTRPRET.C", 2690
}

// 0x461E98
static void insertProgram(Program* program)
{
    ProgramListNode* programListNode = (ProgramListNode*)mymalloc(sizeof(*programListNode), __FILE__, __LINE__); // .\\int\\INTRPRET.C, 2674
    programListNode->program = program;
    programListNode->next = head;
    programListNode->prev = NULL;

    if (head != NULL) {
        head->prev = programListNode;
    }

    head = programListNode;
}

// 0x461E90
void Program::run()
{
    flags |= PROGRAM_FLAG_0x02;
    insertProgram(this);
}

// 0x461ED8
Program* runScript(char* name)
{
    Program* program;

    // NOTE: Uninline.
    program = allocateProgram(interpretMangleName(name));
    if (program != NULL) {
        // NOTE: Uninline.
        program->run();
        program->interpret(24);
    }

    return program;
}

// 0x461F18
void interpretSetCPUBurstSize(int value)
{
    if (value < 1) {
        value = 1;
    }

    cpuBurstSize = value;
}

// 0x461F28
void updatePrograms()
{
    ProgramListNode* curr = head;
    while (curr != NULL) {
        ProgramListNode* next = curr->next;
        if (curr->program != NULL) {
            curr->program->interpret(cpuBurstSize);
        }
        if (curr->program->exited) {
            removeProgList(curr);
        }
        curr = next;
    }
    doEvents();
    updateIntLib();
}

// 0x461F74
void clearPrograms()
{
    ProgramListNode* curr = head;
    while (curr != NULL) {
        ProgramListNode* next = curr->next;
        removeProgList(curr);
        curr = next;
    }
}

// 0x461F90
void clearTopProgram()
{
    ProgramListNode* next;

    next = head->next;
    removeProgList(head);
    head = next;
}

// 0x461FA8
char** getProgramList(int* programListLengthPtr)
{
    char** programList;
    int programListLength;
    int index;
    int it;
    ProgramListNode* programListNode;

    index = 0;
    programListLength = 0;

    for (it = 1; it <= 2; it++) {
        programListNode = head;
        while (programListNode != NULL) {
            if (it == 1) {
                programListLength++;
            } else if (it == 2) {
                if (index < programListLength) {
                    programList[index++] = mystrdup(programListNode->program->name, __FILE__, __LINE__); // "..\int\INTRPRET.C", 2781
                }
            }
            programListNode = programListNode->next;
        }

        if (it == 1) {
            programList = (char**)mymalloc(sizeof(*programList) * programListLength, __FILE__, __LINE__); // "..\int\INTRPRET.C", 2788
        }
    }

    if (programListLengthPtr != NULL) {
        *programListLengthPtr = programListLength;
    }

    return programList;
}

// 0x462058
void freeProgramList(char** programList, int programListLength)
{
    int index;

    if (programList != NULL) {
        for (index = 0; index < programListLength; index++) {
            if (programList[index] != NULL) {
                myfree(programList[index], __FILE__, __LINE__); // "..\int\INTRPRET.C", 2802
            }
        }
    }

    myfree(programList, __FILE__, __LINE__); // "..\int\INTRPRET.C", 2805
}

// 0x4620A4
void interpretAddFunc(int opcode, OpcodeHandler* handler)
{
    int index = opcode & 0x3FFF;
    if (index >= OPCODE_MAX_COUNT) {
        printf("Too many opcodes!\n");
        exit(1);
    }

    opTable[index] = handler;
}

// 0x4620D4
void interpretSetFilenameFunc(InterpretMangleFunc* func)
{
    filenameFunc = func;
}

// 0x4620DC
void interpretSuspendEvents()
{
    suspendEvents++;
    if (suspendEvents == 1) {
        suspendTime = timerFunc();
    }
}

// 0x4620FC
void interpretResumeEvents()
{
    int counter;
    ProgramListNode* programListNode;
    unsigned int time;
    int procedureCount;
    int procedureIndex;
    unsigned char* procedurePtr;

    counter = suspendEvents;
    if (suspendEvents != 0) {
        suspendEvents--;
        if (counter == 1) {
            programListNode = head;
            time = 1000 * (timerFunc() - suspendTime) / timerTick;
            while (programListNode != NULL) {
                procedureCount = fetchLong(programListNode->program->procedures, 0);
                procedurePtr = programListNode->program->procedures + 4;
                for (procedureIndex = 0; procedureIndex < procedureCount; procedureIndex++) {
                    if ((fetchLong(procedurePtr, 4) & PROCEDURE_FLAG_TIMED) != 0) {
                        storeLong(fetchLong(procedurePtr, 8) + time, procedurePtr, 8);
                    }
                }
                programListNode = programListNode->next;
                procedurePtr += sizeof(Procedure);
            }
        }
    }
}

// 0x46224C
int interpretSaveProgramState()
{
    return 0;
}

// 0x46224C
int interpretLoadProgramState()
{
    return 0;
}

void Program::stackPushValue(ProgramValue& programValue)
{
    if (stackValues->size() > STACK_SIZE) {
        interpretError("programStackPushValue: Stack overflow.");
    }

    stackValues->push_back(programValue);
}

void Program::stackPushInteger(int value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_INT;
    programValue.integerValue = value;
    stackPushValue(programValue);
}

void Program::stackPushFloat(float value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_FLOAT;
    programValue.floatValue = value;
    stackPushValue(programValue);
}

void Program::stackPushString(char* value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_DYNAMIC_STRING;
    programValue.integerValue = addString(value);
    stackPushValue(programValue);
}

void Program::stackPushPointer(void* value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_PTR;
    programValue.pointerValue = value;
    stackPushValue(programValue);
}

ProgramValue Program::stackPopValue()
{
    if (stackValues->empty()) {
        interpretError("programStackPopValue: Stack underflow.");
    }

    ProgramValue programValue = stackValues->back();
    stackValues->pop_back();

    return programValue;
}

int Program::stackPopInteger()
{
    ProgramValue programValue = stackPopValue();
    if (programValue.opcode != VALUE_TYPE_INT) {
        interpretError("integer expected, got %x", programValue.opcode);
    }
    return programValue.integerValue;
}

float Program::stackPopFloat()
{
    ProgramValue programValue = stackPopValue();
    if (programValue.opcode != VALUE_TYPE_INT) {
        interpretError("float expected, got %x", programValue.opcode);
    }
    return programValue.floatValue;
}

char* Program::stackPopString()
{
    ProgramValue programValue = stackPopValue();
    if ((programValue.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING) {
        interpretError("string expected, got %x", programValue.opcode);
    }
    return getString(programValue.opcode, programValue.integerValue);
}

void* Program::stackPopPointer()
{
    ProgramValue programValue = stackPopValue();

    // There are certain places in the scripted code where they refer to
    // uninitialized exported variables designed to hold objects (pointers).
    // If this is one theses places simply return NULL.
    if (programValue.opcode == VALUE_TYPE_INT && programValue.integerValue == 0) {
        return NULL;
    }

    if (programValue.opcode != VALUE_TYPE_PTR) {
        interpretError("pointer expected, got %x", programValue.opcode);
    }
    return programValue.pointerValue;
}

void Program::returnStackPushValue(ProgramValue& programValue)
{
    if (returnStackValues->size() > STACK_SIZE) {
        interpretError("programReturnStackPushValue: Stack overflow.");
    }

    returnStackValues->push_back(programValue);
}

void Program::returnStackPushInteger(int value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_INT;
    programValue.integerValue = value;
    returnStackPushValue(programValue);
}

void Program::returnStackPushPointer(void* value)
{
    ProgramValue programValue;
    programValue.opcode = VALUE_TYPE_PTR;
    programValue.pointerValue = value;
    returnStackPushValue(programValue);
}

ProgramValue Program::returnStackPopValue()
{
    if (returnStackValues->empty()) {
        interpretError("programReturnStackPopValue: Stack underflow.");
    }

    ProgramValue programValue = returnStackValues->back();
    returnStackValues->pop_back();

    return programValue;
}

int Program::returnStackPopInteger()
{
    ProgramValue programValue = returnStackPopValue();
    return programValue.integerValue;
}

void* Program::returnStackPopPointer()
{
    ProgramValue programValue = returnStackPopValue();
    return programValue.pointerValue;
}

bool ProgramValue::isEmpty()
{
    switch (opcode) {
    case VALUE_TYPE_INT:
    case VALUE_TYPE_STRING:
        return integerValue == 0;
    case VALUE_TYPE_FLOAT:
        return floatValue == 0.0;
    case VALUE_TYPE_PTR:
        return pointerValue == nullptr;
    }

    // Should be unreachable.
    return true;
}

} // namespace fallout
