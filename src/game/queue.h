#pragma once

#include "game/object_types.h"
#include "plib/db/db.h"

namespace fallout {

enum class EventType : int {
    Drug = 0,
    Knockout = 1,
    Withdrawal = 2,
    Script = 3,
    GameTime = 4,
    Poison = 5,
    Radiation = 6,
    Flare = 7,
    Explosion = 8,
    ItemTrickle = 9,
    Sneak = 10,
    ExplosionFailure = 11,
    MapUpdateEvent = 12,
    Count = 13,
};

inline constexpr int EVENT_TYPE_DRUG = static_cast<int>(EventType::Drug);
inline constexpr int EVENT_TYPE_KNOCKOUT = static_cast<int>(EventType::Knockout);
inline constexpr int EVENT_TYPE_WITHDRAWAL = static_cast<int>(EventType::Withdrawal);
inline constexpr int EVENT_TYPE_SCRIPT = static_cast<int>(EventType::Script);
inline constexpr int EVENT_TYPE_GAME_TIME = static_cast<int>(EventType::GameTime);
inline constexpr int EVENT_TYPE_POISON = static_cast<int>(EventType::Poison);
inline constexpr int EVENT_TYPE_RADIATION = static_cast<int>(EventType::Radiation);
inline constexpr int EVENT_TYPE_FLARE = static_cast<int>(EventType::Flare);
inline constexpr int EVENT_TYPE_EXPLOSION = static_cast<int>(EventType::Explosion);
inline constexpr int EVENT_TYPE_ITEM_TRICKLE = static_cast<int>(EventType::ItemTrickle);
inline constexpr int EVENT_TYPE_SNEAK = static_cast<int>(EventType::Sneak);
inline constexpr int EVENT_TYPE_EXPLOSION_FAILURE = static_cast<int>(EventType::ExplosionFailure);
inline constexpr int EVENT_TYPE_MAP_UPDATE_EVENT = static_cast<int>(EventType::MapUpdateEvent);
inline constexpr int EVENT_TYPE_COUNT = static_cast<int>(EventType::Count);

struct DrugEffectEvent {
    int drugPid;
    int stats[3];
    int modifiers[3];
};

struct WithdrawalEvent {
    int field_0;
    int pid;
    int perk;
};

struct ScriptEvent {
    int sid;
    int fixedParam;
};

struct RadiationEvent {
    int radiationLevel;
    int isHealing;
};

struct AmbientSoundEffectEvent {
    int ambientSoundEffectIndex;
};

using QueueEventHandler = int(Object* owner, void* data);
using QueueEventDataFreeProc = void(void* data);
using QueueEventDataReadProc = int(DB_FILE* stream, void** dataPtr);
using QueueEventDataWriteProc = int(DB_FILE* stream, void* data);

struct EventTypeDescription {
    QueueEventHandler* handlerProc;
    QueueEventDataFreeProc* freeProc;
    QueueEventDataReadProc* readProc;
    QueueEventDataWriteProc* writeProc;
    bool field_10;
    QueueEventHandler* field_14;
};

// ---------------------------------------------------------------------------
// EventQueue — manages time-ordered queue of game events
// ---------------------------------------------------------------------------
class EventQueue {
public:
    static EventQueue& instance();

    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    void init();
    [[nodiscard]] int reset();
    int exit();
    [[nodiscard]] int load(DB_FILE* stream);
    [[nodiscard]] int save(DB_FILE* stream);
    [[nodiscard]] int add(int delay, Object* owner, void* data, int eventType);
    [[nodiscard]] int remove(Object* owner);
    [[nodiscard]] int removeThis(Object* owner, int eventType);
    [[nodiscard]] bool find(Object* owner, int eventType);
    [[nodiscard]] int process();
    void clear();
    void clearType(int eventType, QueueEventHandler* fn);
    [[nodiscard]] int nextTime();
    void leavingMap();

    /// Event type dispatch table (public for legacy handler registration)
    EventTypeDescription eventTypes[EVENT_TYPE_COUNT];

private:
    EventQueue() = default;

    struct Node {
        int time;
        int type;
        Object* owner;
        void* data;
        Node* next;
    };

    Node* head_ = nullptr;
};

// Legacy free-function wrappers
void queue_init();
int queue_reset();
int queue_exit();
int queue_load(DB_FILE* stream);
int queue_save(DB_FILE* stream);
int queue_add(int delay, Object* owner, void* data, int eventType);
int queue_remove(Object* owner);
int queue_remove_this(Object* owner, int eventType);
bool queue_find(Object* owner, int eventType);
int queue_process();
void queue_clear();
void queue_clear_type(int eventType, QueueEventHandler* fn);
int queue_next_time();
void queue_leaving_map();

} // namespace fallout
