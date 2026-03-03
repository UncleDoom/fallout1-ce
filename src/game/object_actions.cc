#include "game/object_actions.h"

#include <cstdio>
#include <cstring>

#include "game/anim.h"
#include "game/combat.h"
#include "game/critter.h"
#include "game/display.h"
#include "game/game.h"
#include "game/gsound.h"
#include "game/intface.h"
#include "game/inventry.h"
#include "game/item.h"
#include "game/map.h"
#include "game/message_helpers.h"
#include "game/object.h"
#include "game/object_lock_state.h"
#include "game/palette.h"
#include "game/perk.h"
#include "game/proto.h"
#include "game/proto_types.h"
#include "game/queue.h"
#include "game/roll.h"
#include "game/scripts.h"
#include "game/skill.h"
#include "game/stat.h"
#include "game/tile.h"
#include "plib/color/color.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/rect.h"

namespace fallout {

// =========================================================================
// Construction
// =========================================================================

ObjectActions::ObjectActions(Object* actor)
    : actor_(actor)
{
}

// =========================================================================
// Look / Examine
// =========================================================================

// 0x48A1FC
int ObjectActions::lookAt(Object* target)
{
    return lookAtFunc(target, display_print);
}

// 0x48A20C
int ObjectActions::lookAtFunc(Object* target, void (*fn)(char* string))
{
    if (critter_is_dead(actor_)) {
        return -1;
    }

    if (FID_TYPE(target->fid) == OBJ_TYPE_TILE) {
        return -1;
    }

    Proto* proto;
    if (proto_ptr(target->pid, &proto) == -1) {
        return -1;
    }

    bool errOccurred = false;
    auto result = try_script_override(target, actor_, SCRIPT_PROC_LOOK_AT, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (!result.overridden) {
        MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file,
            PID_TYPE(target->pid) == OBJ_TYPE_CRITTER && critter_is_dead(target) ? 491 + roll_random(0, 1) : 490);
        if (is_valid_message(messageListItem)) {
            const char* objectName = object_name(target);
            char formattedText[260];
            snprintf(formattedText, sizeof(formattedText), messageListItem.text, objectName);
            fn(formattedText);
        }
    }

    return -1;
}

// 0x48A338
int ObjectActions::examine(Object* target)
{
    return examineFunc(target, display_print);
}

// ---- Examine sub-helpers ------------------------------------------------

void ObjectActions::examineWithAwareness(Object* target, char* buf, size_t bufSize, void (*fn)(char*))
{
    MessageListItem hpMessageListItem;

    if (critter_body_type(target) != BODY_TYPE_BIPED) {
        hpMessageListItem = find_message_or_panic(proto_main_msg_file, 537);
    } else {
        hpMessageListItem = find_message_or_panic(proto_main_msg_file, 535 + stat_level(target, STAT_GENDER));
    }

    Object* item2 = inven_right_hand(target);
    if (item2 != nullptr && item_get_type(item2) != ITEM_TYPE_WEAPON) {
        item2 = nullptr;
    }

    if (item2 != nullptr) {
        MessageListItem weaponMessageListItem;
        if (item_w_caliber(item2) != 0) {
            weaponMessageListItem = find_message_or_panic(proto_main_msg_file, 547);
        } else {
            weaponMessageListItem = find_message_or_panic(proto_main_msg_file, 546);
        }

        char format[80];
        snprintf(format, sizeof(format), "%s%s", hpMessageListItem.text, weaponMessageListItem.text);

        if (item_w_caliber(item2) != 0) {
            const int ammoTypePid = item_w_ammo_pid(item2);
            const char* ammoName = proto_name(ammoTypePid);
            const int ammoCapacity = item_w_max_ammo(item2);
            const int ammoQuantity = item_w_curr_ammo(item2);
            const char* weaponName = object_name(item2);
            const int maxHp = stat_level(target, STAT_MAXIMUM_HIT_POINTS);
            const int curHp = stat_level(target, STAT_CURRENT_HIT_POINTS);
            snprintf(buf, bufSize, format, curHp, maxHp, weaponName, ammoQuantity, ammoCapacity, ammoName);
        } else {
            const char* weaponName = object_name(item2);
            const int maxHp = stat_level(target, STAT_MAXIMUM_HIT_POINTS);
            const int curHp = stat_level(target, STAT_CURRENT_HIT_POINTS);
            snprintf(buf, bufSize, format, curHp, maxHp, weaponName);
        }
    } else {
        MessageListItem endingMessageListItem;
        if (critter_is_crippled(target)) {
            endingMessageListItem = find_message_or_panic(proto_main_msg_file, 544);
        } else {
            endingMessageListItem = find_message_or_panic(proto_main_msg_file, 545);
        }

        const int maxHp = stat_level(target, STAT_MAXIMUM_HIT_POINTS);
        const int curHp = stat_level(target, STAT_CURRENT_HIT_POINTS);
        snprintf(buf, bufSize, hpMessageListItem.text, curHp, maxHp);
        strcat(buf, endingMessageListItem.text);
    }
}

void ObjectActions::examineHealthStatus(Object* target, char* buf, size_t bufSize)
{
    int v12 = 0;
    if (critter_is_crippled(target)) {
        v12 -= 2;
    }

    const int maxHp = stat_level(target, STAT_MAXIMUM_HIT_POINTS);
    const int curHp = stat_level(target, STAT_CURRENT_HIT_POINTS);

    int v16;
    if (curHp <= 0 || critter_is_dead(target)) {
        v16 = 0;
    } else if (curHp == maxHp) {
        v16 = 4;
    } else {
        v16 = (curHp * 3) / maxHp + 1;
    }

    MessageListItem hpMessageListItem = find_message_or_panic(proto_main_msg_file, 500 + v16);

    if (v16 > 4) {
        // Error: lookup_val out of range
        MessageListItem errItem = find_message_or_panic(proto_main_msg_file, 550);
        debug_printf(errItem.text);
        return;
    }

    if (target == obj_dude) {
        // You look %s
        MessageListItem v66 = find_message_or_panic(proto_main_msg_file, 520 + v12);
        snprintf(buf, bufSize, v66.text, hpMessageListItem.text);
    } else {
        MessageListItem v63 = find_message_or_panic(proto_main_msg_file, 522 + stat_level(target, STAT_GENDER));
        snprintf(buf, bufSize, v63.text, hpMessageListItem.text);
    }
}

void ObjectActions::appendCrippledStatus(Object* target, char* buf, size_t bufSize)
{
    if (!critter_is_crippled(target)) return;

    const int maxHp = stat_level(target, STAT_MAXIMUM_HIT_POINTS);
    const int curHp = stat_level(target, STAT_CURRENT_HIT_POINTS);

    int msgNum = maxHp >= curHp ? 531 : 530;
    if (target == obj_dude) {
        msgNum += 2;
    }

    MessageListItem v63 = find_message_or_panic(proto_main_msg_file, msgNum);
    strcat(buf, v63.text);
}

void ObjectActions::examineWeaponAmmo(Object* target, char* buf, size_t bufSize, void (*fn)(char*))
{
    int itemType = item_get_type(target);
    if (itemType == ITEM_TYPE_WEAPON) {
        if (item_w_caliber(target) != 0) {
            MessageListItem weaponMessageListItem = find_message_or_panic(proto_main_msg_file, 526);

            int ammoTypePid = item_w_ammo_pid(target);
            const char* ammoName = proto_name(ammoTypePid);
            int ammoCapacity = item_w_max_ammo(target);
            int ammoQuantity = item_w_curr_ammo(target);
            snprintf(buf, bufSize, weaponMessageListItem.text, ammoQuantity, ammoCapacity, ammoName);
            fn(buf);
        }
    }
}

// 0x48A348
int ObjectActions::examineFunc(Object* target, void (*fn)(char* string))
{
    if (critter_is_dead(actor_)) {
        return -1;
    }

    if (FID_TYPE(target->fid) == OBJ_TYPE_TILE) {
        return -1;
    }

    bool errOccurred = false;
    auto result = try_script_override(target, actor_, SCRIPT_PROC_DESCRIPTION, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (!result.overridden) {
        char* description = object_description(target);
        if (description != nullptr && strcmp(description, proto_none_str) == 0) {
            description = nullptr;
        }

        if (description == nullptr || *description == '\0') {
            MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file, 493);
            fn(messageListItem.text);
        } else {
            if (PID_TYPE(target->pid) != OBJ_TYPE_CRITTER || !critter_is_dead(target)) {
                fn(description);
            }
        }
    }

    if (actor_ == nullptr || actor_ != obj_dude) {
        return 0;
    }

    char formattedText[260];

    int type = PID_TYPE(target->pid);
    if (type == OBJ_TYPE_CRITTER) {
        if (target != obj_dude && perk_level(PERK_AWARENESS) && !critter_is_dead(target)) {
            examineWithAwareness(target, formattedText, sizeof(formattedText), fn);
        } else {
            examineHealthStatus(target, formattedText, sizeof(formattedText));
        }

        appendCrippledStatus(target, formattedText, sizeof(formattedText));
        fn(formattedText);
    } else if (type == OBJ_TYPE_ITEM) {
        examineWeaponAmmo(target, formattedText, sizeof(formattedText), fn);
    }

    return 0;
}

// =========================================================================
// Inventory
// =========================================================================

// 0x48AA3C
int ObjectActions::pickup(Object* item)
{
    bool errOccurred = false;
    auto result = try_script_override(item, actor_, SCRIPT_PROC_PICKUP, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (!result.overridden) {
        int rc;
        if (item->pid == PROTO_ID_MONEY) {
            int amount = item_caps_get_amount(item);
            if (amount <= 0) {
                amount = 1;
            }
            rc = item_add_mult(actor_, item, amount);
            if (rc == 0) {
                item_caps_set_amount(item, 0);
            }
        } else {
            rc = item_add_mult(actor_, item, 1);
        }

        if (rc == 0) {
            Rect rect;
            obj_disconnect(item, &rect);
            tile_refresh_rect(&rect, item->elevation);
        } else {
            display_message(proto_main_msg_file, 905);
        }
    }

    return 0;
}

// 0x48AB24
int ObjectActions::removeFromInven(Object* item)
{
    Rect updatedRect;
    int fid;
    int v11 = 0;

    if (inven_right_hand(actor_) == item) {
        if (actor_ != obj_dude || intface_is_item_right_hand()) {
            fid = art_id(OBJ_TYPE_CRITTER, actor_->fid & 0xFFF, FID_ANIM_TYPE(actor_->fid), 0, actor_->rotation);
            obj_change_fid(actor_, fid, &updatedRect);
            v11 = 2;
        } else {
            v11 = 1;
        }
    } else if (inven_left_hand(actor_) == item) {
        if (actor_ == obj_dude && !intface_is_item_right_hand()) {
            fid = art_id(OBJ_TYPE_CRITTER, actor_->fid & 0xFFF, FID_ANIM_TYPE(actor_->fid), 0, actor_->rotation);
            obj_change_fid(actor_, fid, &updatedRect);
            v11 = 2;
        } else {
            v11 = 1;
        }
    } else if (inven_worn(actor_) == item) {
        if (actor_ == obj_dude) {
            int v5 = 1;

            Proto* proto;
            if (proto_ptr(0x1000000, &proto) != -1) {
                v5 = proto->fid;
            }

            fid = art_id(OBJ_TYPE_CRITTER, v5, FID_ANIM_TYPE(actor_->fid), (actor_->fid & 0xF000) >> 12, actor_->rotation);
            obj_change_fid(actor_, fid, &updatedRect);
            v11 = 3;
        }
    }

    int rc = item_remove_mult(actor_, item, 1);

    if (v11 >= 2) {
        tile_refresh_rect(&updatedRect, actor_->elevation);
    }

    if (v11 <= 2 && actor_ == obj_dude) {
        intface_update_items(false);
    }

    return rc;
}

// 0x48AC94
int ObjectActions::drop(Object* item)
{
    if (item == nullptr) {
        return -1;
    }

    bool errOccurred = false;
    auto result = try_script_override(item, actor_, SCRIPT_PROC_DROP, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (result.overridden) {
        return 0;
    }

    if (removeFromInven(item) == 0) {
        Object* owner = obj_top_environment(actor_);
        if (owner == nullptr) {
            owner = actor_;
        }

        Rect updatedRect;
        obj_connect(item, owner->tile, owner->elevation, &updatedRect);
        tile_refresh_rect(&updatedRect, owner->elevation);
    }

    return 0;
}

// =========================================================================
// Private item-use helpers
// =========================================================================

// Read a book.
// 0x48AD88
int ObjectActions::useBook(Object* book)
{
    int messageId = -1;
    int skill;

    switch (book->pid) {
    case PROTO_ID_BIG_BOOK_OF_SCIENCE:
        messageId = 802;
        skill = SKILL_SCIENCE;
        break;
    case PROTO_ID_DEANS_ELECTRONICS:
        messageId = 803;
        skill = SKILL_REPAIR;
        break;
    case PROTO_ID_FIRST_AID_BOOK:
        messageId = 804;
        skill = SKILL_FIRST_AID;
        break;
    case PROTO_ID_SCOUT_HANDBOOK:
        messageId = 806;
        skill = SKILL_OUTDOORSMAN;
        break;
    case PROTO_ID_GUNS_AND_BULLETS:
        messageId = 805;
        skill = SKILL_SMALL_GUNS;
        break;
    }

    if (messageId == -1) {
        return -1;
    }

    if (isInCombat()) {
        // You cannot do that in combat.
        display_message(proto_main_msg_file, 902);
        return 0;
    }

    int increase = (100 - skill_level(obj_dude, skill)) / 10;
    if (increase <= 0) {
        messageId = 801;
    } else {
        for (int i = 0; i < increase; i++) {
            if (stat_pc_set(PC_STAT_UNSPENT_SKILL_POINTS, stat_pc_get(PC_STAT_UNSPENT_SKILL_POINTS) + 1) == 0) {
                skill_inc_point(obj_dude, skill);
            }
        }
    }

    palette_fade_to(black_palette);

    int intelligence = stat_level(obj_dude, STAT_INTELLIGENCE);
    inc_game_time_in_seconds(3600 * (11 - intelligence));

    scr_exec_map_update_scripts();

    palette_fade_to(cmap);

    // You read the book.
    display_message(proto_main_msg_file, 800);
    display_message(proto_main_msg_file, messageId);

    return 1;
}

// Light a flare.
// 0x48AF24
int ObjectActions::useFlare(Object* flare)
{
    if (flare->pid != PROTO_ID_FLARE) {
        return -1;
    }

    if ((flare->flags & OBJECT_USED) != 0) {
        // The flare is already lit.
        display_message(proto_main_msg_file, 588);
    } else {
        // You light the flare.
        display_message(proto_main_msg_file, 587);

        flare->pid = PROTO_ID_LIT_FLARE;

        obj_set_light(flare, 8, 0x10000, nullptr);
        queue_add(72000, flare, nullptr, EVENT_TYPE_FLARE);
    }

    return 0;
}

// 0x48AFC8
int ObjectActions::useRadio(Object* item)
{
    Script* scr;
    int sid = item->sid;

    if (sid == -1) {
        return -1;
    }

    scr_set_objs(sid, obj_dude, item);
    exec_script_proc(sid, SCRIPT_PROC_USE);

    if (scr_ptr(sid, &scr) == -1) {
        return -1;
    }

    return 0;
}

// 0x48B01C
int ObjectActions::useExplosive(Object* explosive)
{
    int pid = explosive->pid;
    if (pid != PROTO_ID_DYNAMITE_I
        && pid != PROTO_ID_PLASTIC_EXPLOSIVES_I
        && pid != PROTO_ID_DYNAMITE_II
        && pid != PROTO_ID_PLASTIC_EXPLOSIVES_II) {
        return -1;
    }

    if ((explosive->flags & OBJECT_USED) != 0) {
        // The timer is already ticking.
        display_message(proto_main_msg_file, 590);
    } else {
        int seconds = inven_set_timer(explosive);
        if (seconds != -1) {
            // You set the timer.
            display_message(proto_main_msg_file, 589);

            if (pid == PROTO_ID_DYNAMITE_I) {
                explosive->pid = PROTO_ID_DYNAMITE_II;
            } else if (pid == PROTO_ID_PLASTIC_EXPLOSIVES_I) {
                explosive->pid = PROTO_ID_PLASTIC_EXPLOSIVES_II;
            }

            int delay = 10 * seconds;
            int roll = skill_result(obj_dude, SKILL_TRAPS, 0, nullptr);

            int eventType;
            switch (roll) {
            case ROLL_CRITICAL_FAILURE:
                delay = 0;
                eventType = EVENT_TYPE_EXPLOSION_FAILURE;
                break;
            case ROLL_FAILURE:
                eventType = EVENT_TYPE_EXPLOSION_FAILURE;
                delay /= 2;
                break;
            default:
                eventType = EVENT_TYPE_EXPLOSION;
                break;
            }

            queue_add(delay, explosive, nullptr, eventType);
        }
    }

    return 0;
}

// 0x48B21C
int ObjectActions::defaultUseItem(Object* target, Object* item)
{
    char formattedText[90];

    int rc;
    switch (item_get_type(item)) {
    case ITEM_TYPE_DRUG:
        if (PID_TYPE(target->pid) != OBJ_TYPE_CRITTER) {
            if (actor_ == obj_dude) {
                // That does nothing
                display_message(proto_main_msg_file, 582);
            }
            return -1;
        }

        if (critter_is_dead(target)) {
            // 583-586: various "already dead" messages
            display_message(proto_main_msg_file, 583 + roll_random(0, 3));
            return -1;
        }

        rc = item_d_take_drug(target, item);

        if (actor_ == obj_dude && target != obj_dude) {
            // 580: You use the %s.
            // 581: You use the %s on %s.
            MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file, 580 + (target != obj_dude));
            if (!is_valid_message(messageListItem)) {
                return -1;
            }
            snprintf(formattedText, sizeof(formattedText), messageListItem.text, object_name(item), object_name(target));
            display_print(formattedText);
        }

        if (target == obj_dude) {
            intface_update_hit_points(true);
        }

        return rc;
    case ITEM_TYPE_WEAPON:
    case ITEM_TYPE_MISC:
        rc = useFlare(item);
        if (rc == 0) {
            return 0;
        }
        break;
    }

    MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file, 582);
    if (is_valid_message(messageListItem)) {
        snprintf(formattedText, sizeof(formattedText), "%s", messageListItem.text);
        display_print(formattedText);
    }
    return -1;
}

// =========================================================================
// Public item usage
// =========================================================================

// 0x49BF38
int ObjectActions::protinstUseItem(Object* item)
{
    int rc;

    switch (item_get_type(item)) {
    case ITEM_TYPE_DRUG:
        rc = -1;
        break;
    case ITEM_TYPE_WEAPON:
    case ITEM_TYPE_MISC:
        rc = useBook(item);
        if (rc != -1) break;

        rc = useFlare(item);
        if (rc == 0) break;

        rc = useRadio(item);
        if (rc == 0) break;

        rc = useExplosive(item);
        if (rc == 0) break;

        if (item_m_uses_charges(item)) {
            rc = item_m_use_charged_item(actor_, item);
            if (rc == 0) break;
        }
        // FALLTHROUGH
    default:
        // That does nothing
        display_message(proto_main_msg_file, 582);
        rc = -1;
    }

    return rc;
}

// 0x48B1DC
int ObjectActions::useItem(Object* item)
{
    int rc = protinstUseItem(item);
    if (rc == 1) {
        // Destroy consumed item — use free function to avoid circular dep
        // (obj_destroy is in ObjectInstance).
        // We inline the destruction here directly.
        if (item->owner != nullptr) {
            ObjectActions(item->owner).removeFromInven(item);
        }
        queue_remove(item);
        Rect rect;
        int elev = item->elevation;
        obj_erase_object(item, &rect);
        if (item->owner == nullptr) {
            tile_refresh_rect(&rect, elev);
        }
        rc = 0;
    }

    scr_exec_map_update_scripts();
    return rc;
}

// 0x48B394
int ObjectActions::protinstUseItemOn(Object* target, Object* item)
{
    int messageId = -1;
    int criticalChanceModifier = 0;
    int skill = -1;

    switch (item->pid) {
    case PROTO_ID_DOCTORS_BAG:
        messageId = 900;
        criticalChanceModifier = 20;
        skill = SKILL_DOCTOR;
        break;
    case PROTO_ID_FIRST_AID_KIT:
        messageId = 901;
        criticalChanceModifier = 20;
        skill = SKILL_FIRST_AID;
        break;
    }

    if (skill == -1) {
        Script* script;
        int sid = -1;

        sid = item->sid;
        if (sid == -1) {
            sid = target->sid;
            if (sid == -1) {
                return defaultUseItem(target, item);
            }

            scr_set_objs(sid, actor_, item);
            exec_script_proc(sid, SCRIPT_PROC_USE_OBJ_ON);

            if (scr_ptr(sid, &script) == -1) {
                return -1;
            }

            if (!script->scriptOverrides) {
                return defaultUseItem(target, item);
            }
        } else {
            scr_set_objs(sid, actor_, target);
            exec_script_proc(sid, SCRIPT_PROC_USE_OBJ_ON);

            if (scr_ptr(sid, &script) == -1) {
                return -1;
            }

            if (script->field_28 == 0) {
                sid = target->sid;
                if (sid == -1) {
                    return defaultUseItem(target, item);
                }

                scr_set_objs(sid, actor_, item);
                exec_script_proc(sid, SCRIPT_PROC_USE_OBJ_ON);

                Script* script2;
                if (scr_ptr(sid, &script2) == -1) {
                    return -1;
                }

                if (!script2->scriptOverrides) {
                    return defaultUseItem(target, item);
                }
            }
        }

        return script->field_28;
    }

    if (isInCombat()) {
        if (actor_ == obj_dude) {
            // You cannot do that in combat.
            display_message(proto_main_msg_file, 902);
        }
        return -1;
    }

    if (skill_use(actor_, target, skill, criticalChanceModifier) != 0) {
        return 0;
    }

    if (roll_random(1, 10) != 1) {
        return 0;
    }

    MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file, messageId);
    if (actor_ == obj_dude && is_valid_message(messageListItem)) {
        display_print(messageListItem.text);
    }

    return 1;
}

// 0x48B618
int ObjectActions::useItemOn(Object* target, Object* item)
{
    int rc = protinstUseItemOn(target, item);

    if (rc == 1) {
        // Destroy consumed item (inline destruction)
        if (item->owner != nullptr) {
            ObjectActions(item->owner).removeFromInven(item);
        }
        queue_remove(item);
        Rect rect;
        int elev = item->elevation;
        obj_erase_object(item, &rect);
        if (item->owner == nullptr) {
            tile_refresh_rect(&rect, elev);
        }
        rc = 0;
    }

    scr_exec_map_update_scripts();
    return rc;
}

// =========================================================================
// Scenery interaction
// =========================================================================

// 0x48B64C
int ObjectActions::checkSceneryApCost(Object* target)
{
    if (!isInCombat()) {
        return 0;
    }

    int actionPoints = actor_->data.critter.combat.ap;
    if (actionPoints >= 3) {
        actor_->data.critter.combat.ap = actionPoints - 3;

        if (actor_ == obj_dude) {
            intface_update_move_points(obj_dude->data.critter.combat.ap, combat_free_move);
        }

        return 0;
    }

    MessageListItem messageListItem = find_message_or_debug_print(proto_main_msg_file, 700);
    if (actor_ == obj_dude && is_valid_message(messageListItem)) {
        display_print(messageListItem.text);
    }

    return -1;
}

// 0x48B6C4
int ObjectActions::use(Object* target)
{
    int type = FID_TYPE(target->fid);

    if (actor_ == obj_dude) {
        if (type != OBJ_TYPE_SCENERY) return -1;
    } else {
        if (type != OBJ_TYPE_SCENERY) return 0;
    }

    Proto* sceneryProto;
    if (proto_ptr(target->pid, &sceneryProto) == -1) {
        return -1;
    }

    if (sceneryProto->scenery.type == SCENERY_TYPE_DOOR) {
        return useDoor(target, 0);
    }

    bool errOccurred = false;
    auto result = try_script_override(target, actor_, SCRIPT_PROC_USE, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (!result.overridden) {
        if (actor_ == obj_dude) {
            // You see: %s
            display_formatted_message(proto_main_msg_file, 480, object_name(target));
        }
    }

    scr_exec_map_update_scripts();
    return 0;
}

// 0x48B9C0
int ObjectActions::useDoor(Object* target, int a3)
{
    ObjectLockState lockState(target);

    if (lockState.isLocked()) {
        const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_LOCKED);
        gsound_play_sfx_file(sfx);
    }

    bool errOccurred = false;
    auto result = try_script_override(target, actor_, SCRIPT_PROC_USE, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (!result.overridden) {
        int start;
        int end;
        int step;
        if (target->frame != 0) {
            start = 1;
            end = (a3 == 0) - 1;
            step = -1;
        } else {
            if (target->data.scenery.door.openFlags & 0x01) {
                return -1;
            }
            start = 0;
            end = (a3 != 0) + 1;
            step = 1;
        }

        register_begin(ANIMATION_REQUEST_RESERVED);

        for (int i = start; i != end; i += step) {
            if (i != 0) {
                if (a3 == 0) {
                    register_object_call(target, target, reinterpret_cast<AnimationCallback*>(ObjectLockState::setDoorStateClosed), -1);
                }
                const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_CLOSED);
                register_object_play_sfx(target, sfx, -1);
                register_object_animate_reverse(target, ANIM_STAND, 0);
            } else {
                if (a3 == 0) {
                    register_object_call(target, target, reinterpret_cast<AnimationCallback*>(ObjectLockState::setDoorStateOpen), -1);
                }
                const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_OPEN);
                register_object_play_sfx(target, sfx, -1);
                register_object_animate(target, ANIM_STAND, 0);
            }
        }

        register_object_must_call(target, target, reinterpret_cast<AnimationCallback*>(ObjectLockState::checkDoorState), -1);
        register_end();
    }

    return 0;
}

// 0x48BB50
int ObjectActions::useContainer(Object* target)
{
    if (FID_TYPE(target->fid) != OBJ_TYPE_ITEM) {
        return -1;
    }

    Proto* itemProto;
    if (proto_ptr(target->pid, &itemProto) == -1) {
        return -1;
    }

    if (itemProto->item.type != ITEM_TYPE_CONTAINER) {
        return -1;
    }

    ObjectLockState lockState(target);

    if (lockState.isLocked()) {
        const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_LOCKED);
        gsound_play_sfx_file(sfx);

        if (actor_ == obj_dude) {
            display_message(proto_main_msg_file, 487);
        }

        return -1;
    }

    bool errOccurred = false;
    auto result = try_script_override(target, actor_, SCRIPT_PROC_USE, &errOccurred);
    if (errOccurred) {
        return -1;
    }

    if (result.overridden) {
        return 0;
    }

    register_begin(ANIMATION_REQUEST_RESERVED);

    if (target->frame == 0) {
        const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_OPEN);
        register_object_play_sfx(target, sfx, 0);
        register_object_animate(target, ANIM_STAND, 0);
    } else {
        const char* sfx = gsnd_build_open_sfx_name(target, SCENERY_SOUND_EFFECT_CLOSED);
        register_object_play_sfx(target, sfx, 0);
        register_object_animate_reverse(target, ANIM_STAND, 0);
    }

    register_end();

    if (actor_ == obj_dude) {
        display_formatted_message(proto_main_msg_file,
            target->frame != 0 ? 486 : 485,
            object_name(target));
    }

    return 0;
}

// 0x48BD4C
int ObjectActions::useSkillOn(Object* target, int skill)
{
    ObjectLockState lockState(target);

    if (lockState.isJammed()) {
        if (actor_ == obj_dude) {
            display_message(misc_message_file, 2001);
        }
        return -1;
    }

    Proto* proto;
    if (proto_ptr(target->pid, &proto) == -1) {
        return -1;
    }

    int sid = target->sid;
    bool scriptOverrides = false;

    if (sid != -1) {
        scr_set_objs(sid, actor_, target);
        scr_set_action_num(sid, skill);
        exec_script_proc(sid, SCRIPT_PROC_USE_SKILL_ON);

        Script* script;
        if (scr_ptr(sid, &script) == -1) {
            return -1;
        }

        scriptOverrides = script->scriptOverrides != 0;
    }

    if (!scriptOverrides) {
        skill_use(actor_, target, skill, 0);
    }

    return 0;
}

} // namespace fallout
