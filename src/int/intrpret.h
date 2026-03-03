#pragma once


#include <memory>
#include <setjmp.h>

#include <vector>

#include "game/enum_utils.h"

namespace fallout {

enum class Opcode : int {
    Noop = 0x8000,
    Push = 0x8001,
    EnterCriticalSection = 0x8002,
    LeaveCriticalSection = 0x8003,
    Jump = 0x8004,
    Call = 0x8005,
    CallAt = 0x8006,
    CallWhen = 0x8007,
    Callstart = 0x8008,
    Exec = 0x8009,
    Spawn = 0x800A,
    Fork = 0x800B,
    AToD = 0x800C,
    DToA = 0x800D,
    Exit = 0x800E,
    Detach = 0x800F,
    ExitProgram = 0x8010,
    StopProgram = 0x8011,
    FetchGlobal = 0x8012,
    StoreGlobal = 0x8013,
    FetchExternal = 0x8014,
    StoreExternal = 0x8015,
    ExportVariable = 0x8016,
    ExportProcedure = 0x8017,
    Swap = 0x8018,
    Swapa = 0x8019,
    Pop = 0x801A,
    Dup = 0x801B,
    PopReturn = 0x801C,
    PopExit = 0x801D,
    PopAddress = 0x801E,
    PopFlags = 0x801F,
    PopFlagsReturn = 0x8020,
    PopFlagsExit = 0x8021,
    PopFlagsReturnExtern = 0x8022,
    PopFlagsExitExtern = 0x8023,
    PopFlagsReturnValExtern = 0x8024,
    PopFlagsReturnValExit = 0x8025,
    PopFlagsReturnValExitExtern = 0x8026,
    CheckProcedureArgumentCount = 0x8027,
    LookupProcedureByName = 0x8028,
    PopBase = 0x8029,
    PopToBase = 0x802A,
    PushBase = 0x802B,
    SetGlobal = 0x802C,
    FetchProcedureAddress = 0x802D,
    Dump = 0x802E,
    If = 0x802F,
    While = 0x8030,
    Store = 0x8031,
    Fetch = 0x8032,
    Equal = 0x8033,
    NotEqual = 0x8034,
    LessThanEqual = 0x8035,
    GreaterThanEqual = 0x8036,
    LessThan = 0x8037,
    GreaterThan = 0x8038,
    Add = 0x8039,
    Sub = 0x803A,
    Mul = 0x803B,
    Div = 0x803C,
    Mod = 0x803D,
    And = 0x803E,
    Or = 0x803F,
    BitwiseAnd = 0x8040,
    BitwiseOr = 0x8041,
    BitwiseXor = 0x8042,
    BitwiseNot = 0x8043,
    Floor = 0x8044,
    Not = 0x8045,
    Negate = 0x8046,
    Wait = 0x8047,
    Cancel = 0x8048,
    CancelAll = 0x8049,
    StartCritical = 0x804A,
    EndCritical = 0x804B,
};

inline constexpr int OPCODE_NOOP = static_cast<int>(Opcode::Noop);
inline constexpr int OPCODE_PUSH = static_cast<int>(Opcode::Push);
inline constexpr int OPCODE_ENTER_CRITICAL_SECTION = static_cast<int>(Opcode::EnterCriticalSection);
inline constexpr int OPCODE_LEAVE_CRITICAL_SECTION = static_cast<int>(Opcode::LeaveCriticalSection);
inline constexpr int OPCODE_JUMP = static_cast<int>(Opcode::Jump);
inline constexpr int OPCODE_CALL = static_cast<int>(Opcode::Call);
inline constexpr int OPCODE_CALL_AT = static_cast<int>(Opcode::CallAt);
inline constexpr int OPCODE_CALL_WHEN = static_cast<int>(Opcode::CallWhen);
inline constexpr int OPCODE_CALLSTART = static_cast<int>(Opcode::Callstart);
inline constexpr int OPCODE_EXEC = static_cast<int>(Opcode::Exec);
inline constexpr int OPCODE_SPAWN = static_cast<int>(Opcode::Spawn);
inline constexpr int OPCODE_FORK = static_cast<int>(Opcode::Fork);
inline constexpr int OPCODE_A_TO_D = static_cast<int>(Opcode::AToD);
inline constexpr int OPCODE_D_TO_A = static_cast<int>(Opcode::DToA);
inline constexpr int OPCODE_EXIT = static_cast<int>(Opcode::Exit);
inline constexpr int OPCODE_DETACH = static_cast<int>(Opcode::Detach);
inline constexpr int OPCODE_EXIT_PROGRAM = static_cast<int>(Opcode::ExitProgram);
inline constexpr int OPCODE_STOP_PROGRAM = static_cast<int>(Opcode::StopProgram);
inline constexpr int OPCODE_FETCH_GLOBAL = static_cast<int>(Opcode::FetchGlobal);
inline constexpr int OPCODE_STORE_GLOBAL = static_cast<int>(Opcode::StoreGlobal);
inline constexpr int OPCODE_FETCH_EXTERNAL = static_cast<int>(Opcode::FetchExternal);
inline constexpr int OPCODE_STORE_EXTERNAL = static_cast<int>(Opcode::StoreExternal);
inline constexpr int OPCODE_EXPORT_VARIABLE = static_cast<int>(Opcode::ExportVariable);
inline constexpr int OPCODE_EXPORT_PROCEDURE = static_cast<int>(Opcode::ExportProcedure);
inline constexpr int OPCODE_SWAP = static_cast<int>(Opcode::Swap);
inline constexpr int OPCODE_SWAPA = static_cast<int>(Opcode::Swapa);
inline constexpr int OPCODE_POP = static_cast<int>(Opcode::Pop);
inline constexpr int OPCODE_DUP = static_cast<int>(Opcode::Dup);
inline constexpr int OPCODE_POP_RETURN = static_cast<int>(Opcode::PopReturn);
inline constexpr int OPCODE_POP_EXIT = static_cast<int>(Opcode::PopExit);
inline constexpr int OPCODE_POP_ADDRESS = static_cast<int>(Opcode::PopAddress);
inline constexpr int OPCODE_POP_FLAGS = static_cast<int>(Opcode::PopFlags);
inline constexpr int OPCODE_POP_FLAGS_RETURN = static_cast<int>(Opcode::PopFlagsReturn);
inline constexpr int OPCODE_POP_FLAGS_EXIT = static_cast<int>(Opcode::PopFlagsExit);
inline constexpr int OPCODE_POP_FLAGS_RETURN_EXTERN = static_cast<int>(Opcode::PopFlagsReturnExtern);
inline constexpr int OPCODE_POP_FLAGS_EXIT_EXTERN = static_cast<int>(Opcode::PopFlagsExitExtern);
inline constexpr int OPCODE_POP_FLAGS_RETURN_VAL_EXTERN = static_cast<int>(Opcode::PopFlagsReturnValExtern);
inline constexpr int OPCODE_POP_FLAGS_RETURN_VAL_EXIT = static_cast<int>(Opcode::PopFlagsReturnValExit);
inline constexpr int OPCODE_POP_FLAGS_RETURN_VAL_EXIT_EXTERN = static_cast<int>(Opcode::PopFlagsReturnValExitExtern);
inline constexpr int OPCODE_CHECK_PROCEDURE_ARGUMENT_COUNT = static_cast<int>(Opcode::CheckProcedureArgumentCount);
inline constexpr int OPCODE_LOOKUP_PROCEDURE_BY_NAME = static_cast<int>(Opcode::LookupProcedureByName);
inline constexpr int OPCODE_POP_BASE = static_cast<int>(Opcode::PopBase);
inline constexpr int OPCODE_POP_TO_BASE = static_cast<int>(Opcode::PopToBase);
inline constexpr int OPCODE_PUSH_BASE = static_cast<int>(Opcode::PushBase);
inline constexpr int OPCODE_SET_GLOBAL = static_cast<int>(Opcode::SetGlobal);
inline constexpr int OPCODE_FETCH_PROCEDURE_ADDRESS = static_cast<int>(Opcode::FetchProcedureAddress);
inline constexpr int OPCODE_DUMP = static_cast<int>(Opcode::Dump);
inline constexpr int OPCODE_IF = static_cast<int>(Opcode::If);
inline constexpr int OPCODE_WHILE = static_cast<int>(Opcode::While);
inline constexpr int OPCODE_STORE = static_cast<int>(Opcode::Store);
inline constexpr int OPCODE_FETCH = static_cast<int>(Opcode::Fetch);
inline constexpr int OPCODE_EQUAL = static_cast<int>(Opcode::Equal);
inline constexpr int OPCODE_NOT_EQUAL = static_cast<int>(Opcode::NotEqual);
inline constexpr int OPCODE_LESS_THAN_EQUAL = static_cast<int>(Opcode::LessThanEqual);
inline constexpr int OPCODE_GREATER_THAN_EQUAL = static_cast<int>(Opcode::GreaterThanEqual);
inline constexpr int OPCODE_LESS_THAN = static_cast<int>(Opcode::LessThan);
inline constexpr int OPCODE_GREATER_THAN = static_cast<int>(Opcode::GreaterThan);
inline constexpr int OPCODE_ADD = static_cast<int>(Opcode::Add);
inline constexpr int OPCODE_SUB = static_cast<int>(Opcode::Sub);
inline constexpr int OPCODE_MUL = static_cast<int>(Opcode::Mul);
inline constexpr int OPCODE_DIV = static_cast<int>(Opcode::Div);
inline constexpr int OPCODE_MOD = static_cast<int>(Opcode::Mod);
inline constexpr int OPCODE_AND = static_cast<int>(Opcode::And);
inline constexpr int OPCODE_OR = static_cast<int>(Opcode::Or);
inline constexpr int OPCODE_BITWISE_AND = static_cast<int>(Opcode::BitwiseAnd);
inline constexpr int OPCODE_BITWISE_OR = static_cast<int>(Opcode::BitwiseOr);
inline constexpr int OPCODE_BITWISE_XOR = static_cast<int>(Opcode::BitwiseXor);
inline constexpr int OPCODE_BITWISE_NOT = static_cast<int>(Opcode::BitwiseNot);
inline constexpr int OPCODE_FLOOR = static_cast<int>(Opcode::Floor);
inline constexpr int OPCODE_NOT = static_cast<int>(Opcode::Not);
inline constexpr int OPCODE_NEGATE = static_cast<int>(Opcode::Negate);
inline constexpr int OPCODE_WAIT = static_cast<int>(Opcode::Wait);
inline constexpr int OPCODE_CANCEL = static_cast<int>(Opcode::Cancel);
inline constexpr int OPCODE_CANCEL_ALL = static_cast<int>(Opcode::CancelAll);
inline constexpr int OPCODE_START_CRITICAL = static_cast<int>(Opcode::StartCritical);
inline constexpr int OPCODE_END_CRITICAL = static_cast<int>(Opcode::EndCritical);

enum class ProcedureFlags : unsigned {
    Timed = 0x01,
    Conditional = 0x02,
    Imported = 0x04,
    Exported = 0x08,
    Critical = 0x10,
};

DEFINE_ENUM_FLAG_OPERATORS(ProcedureFlags)

inline constexpr int PROCEDURE_FLAG_TIMED = static_cast<int>(ProcedureFlags::Timed);
inline constexpr int PROCEDURE_FLAG_CONDITIONAL = static_cast<int>(ProcedureFlags::Conditional);
inline constexpr int PROCEDURE_FLAG_IMPORTED = static_cast<int>(ProcedureFlags::Imported);
inline constexpr int PROCEDURE_FLAG_EXPORTED = static_cast<int>(ProcedureFlags::Exported);
inline constexpr int PROCEDURE_FLAG_CRITICAL = static_cast<int>(ProcedureFlags::Critical);


enum class ProgramFlags : unsigned {
    Exited = 0x01,
    Flag0x02 = 0x02,
    Flag0x04 = 0x04,
    Stopped = 0x08,

    // Program is in waiting state with `checkWaitFunc` set.
    IsWaiting = 0x10,
    Flag0x20 = 0x20,
    Flag0x40 = 0x40,
    CriticalSection = 0x80,
    Flag0x0100 = 0x0100,
};

DEFINE_ENUM_FLAG_OPERATORS(ProgramFlags)

inline constexpr int PROGRAM_FLAG_EXITED = static_cast<int>(ProgramFlags::Exited);
inline constexpr int PROGRAM_FLAG_0x02 = static_cast<int>(ProgramFlags::Flag0x02);
inline constexpr int PROGRAM_FLAG_0x04 = static_cast<int>(ProgramFlags::Flag0x04);
inline constexpr int PROGRAM_FLAG_STOPPED = static_cast<int>(ProgramFlags::Stopped);
inline constexpr int PROGRAM_IS_WAITING = static_cast<int>(ProgramFlags::IsWaiting);
inline constexpr int PROGRAM_FLAG_0x20 = static_cast<int>(ProgramFlags::Flag0x20);
inline constexpr int PROGRAM_FLAG_0x40 = static_cast<int>(ProgramFlags::Flag0x40);
inline constexpr int PROGRAM_FLAG_CRITICAL_SECTION = static_cast<int>(ProgramFlags::CriticalSection);
inline constexpr int PROGRAM_FLAG_0x0100 = static_cast<int>(ProgramFlags::Flag0x0100);


enum class RawValueType : int {
    Opcode = 0x8000,
    Int = 0x4000,
    Float = 0x2000,
    StaticString = 0x1000,
    DynamicString = 0x0800,
};

inline constexpr int RAW_VALUE_TYPE_OPCODE = static_cast<int>(RawValueType::Opcode);
inline constexpr int RAW_VALUE_TYPE_INT = static_cast<int>(RawValueType::Int);
inline constexpr int RAW_VALUE_TYPE_FLOAT = static_cast<int>(RawValueType::Float);
inline constexpr int RAW_VALUE_TYPE_STATIC_STRING = static_cast<int>(RawValueType::StaticString);
inline constexpr int RAW_VALUE_TYPE_DYNAMIC_STRING = static_cast<int>(RawValueType::DynamicString);

inline constexpr int VALUE_TYPE_MASK = 0xF7FF;

inline constexpr int VALUE_TYPE_INT = 0xC001;
inline constexpr int VALUE_TYPE_FLOAT = 0xA001;
inline constexpr int VALUE_TYPE_STRING = 0x9001;
inline constexpr int VALUE_TYPE_DYNAMIC_STRING = 0x9801;
inline constexpr int VALUE_TYPE_PTR = 0xE001;

using opcode_t = unsigned short;

struct Procedure {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
};

struct ProgramValue {
    opcode_t opcode;
    union {
        int integerValue;
        float floatValue;
        void* pointerValue;
    };

    bool isEmpty();
};

using ProgramStack = std::vector<ProgramValue>;

class Program;
using InterpretCheckWaitFunc = int(Program* program);

// It's size in original code is 144 (0x8C) bytes due to the different
// size of `jmp_buf`.
class Program {
public:
    char* name;
    unsigned char* data;
    Program* parent;
    Program* child;
    int instructionPointer; // current pos in data
    int framePointer; // saved stack 1 pos - probably beginning of local variables - probably called base
    int basePointer; // saved stack 1 pos - probably beginning of global variables
    unsigned char* staticStrings; // static strings table
    unsigned char* dynamicStrings; // dynamic strings table
    unsigned char* identifiers;
    unsigned char* procedures;
    jmp_buf env;
    unsigned int waitEnd; // end time of timer (field_74 + wait time)
    unsigned int waitStart; // time when wait was called
    int field_78; // time when program begin execution (for the first time)?, -1 - program never executed
    InterpretCheckWaitFunc* checkWaitFunc;
    int flags; // flags
    int windowId;
    bool exited;
    std::unique_ptr<ProgramStack> stackValues;
    std::unique_ptr<ProgramStack> returnStackValues;

    // Methods (converted from free functions)
    void freeProgram();
    char* getString(opcode_t opcode, int offset);
    char* getName(int offset);
    int addString(char* string);
    void interpret(int a2);
    void executeProc(int procedureIndex);
    int findProcedure(const char* name);
    void executeProcedure(int procedureIndex);
    void run();

    void stackPushValue(ProgramValue& programValue);
    void stackPushInteger(int value);
    void stackPushFloat(float value);
    void stackPushString(char* string);
    void stackPushPointer(void* value);

    ProgramValue stackPopValue();
    int stackPopInteger();
    float stackPopFloat();
    char* stackPopString();
    void* stackPopPointer();

    void returnStackPushValue(ProgramValue& programValue);
    void returnStackPushInteger(int value);
    void returnStackPushPointer(void* value);

    ProgramValue returnStackPopValue();
    int returnStackPopInteger();
    void* returnStackPopPointer();
};

using InterpretMangleFunc = char*(char* fileName);
using InterpretOutputFunc = int(char* string);
using InterpretTimerFunc = unsigned int();
using OpcodeHandler = void(Program* program);

void interpretSetTimeFunc(InterpretTimerFunc* timerFunc, int timerTick);
char* interpretMangleName(char* fileName);
void interpretOutputFunc(InterpretOutputFunc* func);
int interpretOutput(const char* format, ...);
void interpretError(const char* format, ...);
Program* allocateProgram(const char* path);
void initInterpreter();
void interpretClose();
void interpretEnableInterpreter(int enabled);
Program* runScript(char* name);
void interpretSetCPUBurstSize(int value);
void updatePrograms();
void clearPrograms();
void clearTopProgram();
char** getProgramList(int* programListLengthPtr);
void freeProgramList(char** programList, int programListLength);
void interpretAddFunc(int opcode, OpcodeHandler* handler);
void interpretSetFilenameFunc(InterpretMangleFunc* func);
void interpretSuspendEvents();
void interpretResumeEvents();
int interpretSaveProgramState();
int interpretLoadProgramState();

} // namespace fallout
