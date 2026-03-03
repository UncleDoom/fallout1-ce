#include "game/queue.h"

#include "game/actions.h"
#include "game/critter.h"
#include "game/display.h"
#include "game/game.h"
#include "game/gsound.h"
#include "game/item.h"
#include "game/map.h"
#include "game/message.h"
#include "game/object.h"
#include "game/perk.h"
#include "game/protinst.h"
#include "game/proto.h"
#include "game/scripts.h"
#include "plib/gnw/memory.h"

namespace fallout {

// ---------------------------------------------------------------------------
// Static event handlers (used in the dispatch table)
// ---------------------------------------------------------------------------
static int queue_destroy_handler(Object* obj, void* data);
static int queue_explode_handler(Object* obj, void* data);
static int queue_explode_exit_handler(Object* obj, void* data);
static int queue_do_explosion(Object* obj, bool premature);
static int queue_premature_handler(Object* obj, void* data);

// ---------------------------------------------------------------------------
// EventQueue implementation
// ---------------------------------------------------------------------------

// static
EventQueue& EventQueue::instance()
{
    static EventQueue inst;
    return inst;
}

// 0x490670
void EventQueue::init()
{
    head_ = nullptr;

    // 0x5076FC — event type dispatch table
    eventTypes[EVENT_TYPE_DRUG] = { item_d_process, mem_free, item_d_load, item_d_save, true, item_d_clear };
    eventTypes[EVENT_TYPE_KNOCKOUT] = { critter_wake_up, nullptr, nullptr, nullptr, true, critter_wake_clear };
    eventTypes[EVENT_TYPE_WITHDRAWAL] = { item_wd_process, mem_free, item_wd_load, item_wd_save, true, item_wd_clear };
    eventTypes[EVENT_TYPE_SCRIPT] = { script_q_process, mem_free, script_q_load, script_q_save, true, nullptr };
    eventTypes[EVENT_TYPE_GAME_TIME] = { gtime_q_process, nullptr, nullptr, nullptr, true, nullptr };
    eventTypes[EVENT_TYPE_POISON] = { critter_check_poison, nullptr, nullptr, nullptr, false, nullptr };
    eventTypes[EVENT_TYPE_RADIATION] = { critter_process_rads, mem_free, critter_load_rads, critter_save_rads, false, nullptr };
    eventTypes[EVENT_TYPE_FLARE] = { queue_destroy_handler, nullptr, nullptr, nullptr, true, queue_destroy_handler };
    eventTypes[EVENT_TYPE_EXPLOSION] = { queue_explode_handler, nullptr, nullptr, nullptr, true, queue_explode_exit_handler };
    eventTypes[EVENT_TYPE_ITEM_TRICKLE] = { item_m_trickle, nullptr, nullptr, nullptr, true, item_m_turn_off_from_queue };
    eventTypes[EVENT_TYPE_SNEAK] = { critter_sneak_check, nullptr, nullptr, nullptr, true, critter_sneak_clear };
    eventTypes[EVENT_TYPE_EXPLOSION_FAILURE] = { queue_premature_handler, nullptr, nullptr, nullptr, true, queue_explode_exit_handler };
    eventTypes[EVENT_TYPE_MAP_UPDATE_EVENT] = { scr_map_q_process, nullptr, nullptr, nullptr, true, nullptr };
}

// 0x490680
int EventQueue::reset()
{
    clear();
    return 0;
}

// 0x490680
int EventQueue::exit()
{
    clear();
    return 0;
}

// 0x490688
int EventQueue::load(DB_FILE* stream)
{
    int count;
    if (stream->freadInt(&count) == -1) {
        return -1;
    }

    head_ = nullptr;

    Node** nextPtr = &head_;

    int rc = 0;
    for (int index = 0; index < count; index += 1) {
        auto* node = static_cast<Node*>(mem_malloc(sizeof(Node)));
        if (node == nullptr) {
            rc = -1;
            break;
        }

        if (stream->freadInt(&(node->time)) == -1) {
            mem_free(node);
            rc = -1;
            break;
        }

        if (stream->freadInt(&(node->type)) == -1) {
            mem_free(node);
            rc = -1;
            break;
        }

        int objectId;
        if (stream->freadInt(&objectId) == -1) {
            mem_free(node);
            rc = -1;
            break;
        }

        Object* obj;
        if (objectId == -2) {
            obj = nullptr;
        } else {
            obj = obj_find_first();
            while (obj != nullptr) {
                obj = inven_find_id(obj, objectId);
                if (obj != nullptr) {
                    break;
                }
                obj = obj_find_next();
            }
        }

        node->owner = obj;

        EventTypeDescription* desc = &(eventTypes[node->type]);
        if (desc->readProc != nullptr) {
            if (desc->readProc(stream, &(node->data)) == -1) {
                mem_free(node);
                rc = -1;
                break;
            }
        } else {
            node->data = nullptr;
        }

        node->next = nullptr;

        *nextPtr = node;
        nextPtr = &(node->next);
    }

    if (rc == -1) {
        while (head_ != nullptr) {
            Node* next = head_->next;

            EventTypeDescription* desc = &(eventTypes[head_->type]);
            if (desc->freeProc != nullptr) {
                desc->freeProc(head_->data);
            }

            mem_free(head_);

            head_ = next;
        }
    }

    return rc;
}

// 0x4907F4
int EventQueue::save(DB_FILE* stream)
{
    int count = 0;
    for (Node* node = head_; node != nullptr; node = node->next) {
        count += 1;
    }

    if (stream->fwriteInt(count) == -1) {
        return -1;
    }

    for (Node* node = head_; node != nullptr; node = node->next) {
        Object* object = node->owner;
        int objectId = object != nullptr ? object->id : -2;

        if (stream->fwriteInt(node->time) == -1) {
            return -1;
        }

        if (stream->fwriteInt(node->type) == -1) {
            return -1;
        }

        if (stream->fwriteInt(objectId) == -1) {
            return -1;
        }

        EventTypeDescription* desc = &(eventTypes[node->type]);
        if (desc->writeProc != nullptr) {
            if (desc->writeProc(stream, node->data) == -1) {
                return -1;
            }
        }
    }

    return 0;
}

// 0x4908A0
int EventQueue::add(int delay, Object* owner, void* data, int eventType)
{
    auto* newNode = static_cast<Node*>(mem_malloc(sizeof(Node)));
    if (newNode == nullptr) {
        return -1;
    }

    int fireTime = game_time() + delay;
    newNode->time = fireTime;
    newNode->type = eventType;
    newNode->owner = owner;
    newNode->data = data;

    if (owner != nullptr) {
        owner->flags |= OBJECT_USED;
    }

    Node** insertPtr = &head_;
    if (head_ != nullptr) {
        Node* cur;
        do {
            cur = *insertPtr;
            if (fireTime < cur->time) {
                break;
            }
            insertPtr = &(cur->next);
        } while (cur->next != nullptr);
    }

    newNode->next = *insertPtr;
    *insertPtr = newNode;

    return 0;
}

// 0x490908
int EventQueue::remove(Object* owner)
{
    Node* node = head_;
    Node** nodePtr = &head_;

    while (node) {
        if (node->owner == owner) {
            Node* temp = node;

            node = node->next;
            *nodePtr = node;

            EventTypeDescription* desc = &(eventTypes[temp->type]);
            if (desc->freeProc != nullptr) {
                desc->freeProc(temp->data);
            }

            mem_free(temp);
        } else {
            nodePtr = &(node->next);
            node = node->next;
        }
    }

    return 0;
}

// 0x490960
int EventQueue::removeThis(Object* owner, int eventType)
{
    Node* node = head_;
    Node** nodePtr = &head_;

    while (node) {
        if (node->owner == owner && node->type == eventType) {
            Node* temp = node;

            node = node->next;
            *nodePtr = node;

            EventTypeDescription* desc = &(eventTypes[temp->type]);
            if (desc->freeProc != nullptr) {
                desc->freeProc(temp->data);
            }

            mem_free(temp);
        } else {
            nodePtr = &(node->next);
            node = node->next;
        }
    }

    return 0;
}

// Returns true if there is at least one event of given type scheduled.
//
// 0x4909BC
bool EventQueue::find(Object* owner, int eventType)
{
    for (Node* node = head_; node != nullptr; node = node->next) {
        if (owner == node->owner && eventType == node->type) {
            return true;
        }
    }

    return false;
}

// 0x4909E4
int EventQueue::process()
{
    int time = game_time();
    int result = 0;

    while (head_ != nullptr) {
        Node* node = head_;
        if (time < node->time || result != 0) {
            break;
        }

        head_ = node->next;

        EventTypeDescription* desc = &(eventTypes[node->type]);
        result = desc->handlerProc(node->owner, node->data);

        if (desc->freeProc != nullptr) {
            desc->freeProc(node->data);
        }

        mem_free(node);
    }

    return result;
}

// 0x490A5C
void EventQueue::clear()
{
    Node* node = head_;
    while (node != nullptr) {
        Node* next = node->next;

        EventTypeDescription* desc = &(eventTypes[node->type]);
        if (desc->freeProc != nullptr) {
            desc->freeProc(node->data);
        }

        mem_free(node);

        node = next;
    }

    head_ = nullptr;
}

// 0x490AA4
void EventQueue::clearType(int eventType, QueueEventHandler* fn)
{
    Node** ptr = &head_;
    Node* curr = *ptr;

    while (curr != nullptr) {
        if (eventType == curr->type) {
            Node* tmp = curr;

            *ptr = curr->next;
            curr = *ptr;

            if (fn != nullptr && fn(tmp->owner, tmp->data) != 1) {
                *ptr = tmp;
                ptr = &(tmp->next);
            } else {
                EventTypeDescription* desc = &(eventTypes[tmp->type]);
                if (desc->freeProc != nullptr) {
                    desc->freeProc(tmp->data);
                }

                mem_free(tmp);
            }
        } else {
            ptr = &(curr->next);
            curr = *ptr;
        }
    }
}

// TODO: Make unsigned.
//
// 0x490B1C
int EventQueue::nextTime()
{
    if (head_ == nullptr) {
        return 0;
    }

    return head_->time;
}

// 0x490C08
void EventQueue::leavingMap()
{
    for (int index = 0; index < EVENT_TYPE_COUNT; index++) {
        if (eventTypes[index].field_10) {
            clearType(index, eventTypes[index].field_14);
        }
    }
}

// ---------------------------------------------------------------------------
// Static event handlers
// ---------------------------------------------------------------------------

// 0x490B30
static int queue_destroy_handler(Object* obj, void* data)
{
    obj_destroy(obj);
    return 1;
}

// 0x490B3C
static int queue_explode_handler(Object* obj, void* data)
{
    return queue_do_explosion(obj, true);
}

// 0x490B44
static int queue_explode_exit_handler(Object* obj, void* data)
{
    return queue_do_explosion(obj, false);
}

// 0x490B48
static int queue_do_explosion(Object* explosive, bool premature)
{
    Object* owner;
    int tile;
    int elevation;
    int min_damage;
    int max_damage;

    owner = obj_top_environment(explosive);
    if (owner) {
        tile = owner->tile;
        elevation = owner->elevation;
    } else {
        tile = explosive->tile;
        elevation = explosive->elevation;
    }

    if (explosive->pid == PROTO_ID_DYNAMITE_I || explosive->pid == PROTO_ID_DYNAMITE_II) {
        // Dynamite
        min_damage = 30;
        max_damage = 50;
    } else {
        // Plastic explosive
        min_damage = 40;
        max_damage = 80;
    }

    if (action_explode(tile, elevation, min_damage, max_damage, obj_dude, premature) == -2) {
        queue_add(50, explosive, nullptr, EVENT_TYPE_EXPLOSION);
    } else {
        obj_destroy(explosive);
    }

    return 1;
}

// 0x490BCC
static int queue_premature_handler(Object* obj, void* data)
{
    MessageListItem msg;

    // Due to your inept handling, the explosive detonates prematurely.
    msg.num = 4000;
    if (misc_message_file.search(&msg)) {
        display_print(msg.text);
    }

    return queue_do_explosion(obj, true);
}

// ---------------------------------------------------------------------------
// Legacy free-function wrappers
// ---------------------------------------------------------------------------

void queue_init() { EventQueue::instance().init(); }
int queue_reset() { return EventQueue::instance().reset(); }
int queue_exit() { return EventQueue::instance().exit(); }
int queue_load(DB_FILE* stream) { return EventQueue::instance().load(stream); }
int queue_save(DB_FILE* stream) { return EventQueue::instance().save(stream); }
int queue_add(int delay, Object* owner, void* data, int eventType) { return EventQueue::instance().add(delay, owner, data, eventType); }
int queue_remove(Object* owner) { return EventQueue::instance().remove(owner); }
int queue_remove_this(Object* owner, int eventType) { return EventQueue::instance().removeThis(owner, eventType); }
bool queue_find(Object* owner, int eventType) { return EventQueue::instance().find(owner, eventType); }
int queue_process() { return EventQueue::instance().process(); }
void queue_clear() { EventQueue::instance().clear(); }
void queue_clear_type(int eventType, QueueEventHandler* fn) { EventQueue::instance().clearType(eventType, fn); }
int queue_next_time() { return EventQueue::instance().nextTime(); }
void queue_leaving_map() { EventQueue::instance().leavingMap(); }

} // namespace fallout
