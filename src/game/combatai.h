#pragma once


#include <cstddef>

#include "game/combat_defs.h"
#include "game/message.h"
#include "game/object_types.h"
#include "game/party.h"
#include "plib/db/db.h"

namespace fallout {

enum class AiMessageType : int {
    Run = 0,
    Move = 1,
    Attack = 2,
    Miss = 3,
    Hit = 4,
};

inline constexpr int AI_MESSAGE_TYPE_RUN = static_cast<int>(AiMessageType::Run);
inline constexpr int AI_MESSAGE_TYPE_MOVE = static_cast<int>(AiMessageType::Move);
inline constexpr int AI_MESSAGE_TYPE_ATTACK = static_cast<int>(AiMessageType::Attack);
inline constexpr int AI_MESSAGE_TYPE_MISS = static_cast<int>(AiMessageType::Miss);
inline constexpr int AI_MESSAGE_TYPE_HIT = static_cast<int>(AiMessageType::Hit);

struct AiPacket {
    char* name;
    int packet_num;
    int max_dist;
    int min_to_hit;
    int min_hp;
    int aggression;
    int hurt_too_much;
    int secondary_freq;
    int called_freq;
    int font;
    int color;
    int outline_color;
    int chance;
    int run_start;
    int move_start;
    int attack_start;
    int miss_start;
    int hit_start[HIT_LOCATION_SPECIFIC_COUNT];
    int last_msg;
};

int combat_ai_init();
void combat_ai_reset();
int combat_ai_exit();
int combat_ai_load(DB_FILE* stream);
int combat_ai_save(DB_FILE* stream);
int combat_ai_num();
char* combat_ai_name(int packetNum);
Object* ai_danger_source(Object* critter);
Object* ai_search_inven(Object* critter, int check_action_points);
void combat_ai_begin(int critters_count, Object** critters);
void combat_ai_over();
Object* combat_ai(Object* critter, Object* target);
bool combatai_want_to_join(Object* critter);
bool combatai_want_to_stop(Object* critter);
int combatai_switch_team(Object* critter, int team);
Object* combat_ai_random_target(Attack* attack);
void combatai_check_retaliation(Object* critter, Object* candidate);
bool is_within_perception(Object* critter1, Object* critter2);
void combatai_refresh_messages();
void combatai_notify_onlookers(Object* critter);
void combatai_delete_critter(Object* critter);

} // namespace fallout
