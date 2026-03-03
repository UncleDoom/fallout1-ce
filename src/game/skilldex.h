#pragma once


namespace fallout {

enum class SkilldexRC : int {
    Error = -1,
    Canceled = 0,
    Sneak = 1,
    Lockpick = 2,
    Steal = 3,
    Traps = 4,
    FirstAid = 5,
    Doctor = 6,
    Science = 7,
    Repair = 8,
};

inline constexpr int SKILLDEX_RC_ERROR = static_cast<int>(SkilldexRC::Error);
inline constexpr int SKILLDEX_RC_CANCELED = static_cast<int>(SkilldexRC::Canceled);
inline constexpr int SKILLDEX_RC_SNEAK = static_cast<int>(SkilldexRC::Sneak);
inline constexpr int SKILLDEX_RC_LOCKPICK = static_cast<int>(SkilldexRC::Lockpick);
inline constexpr int SKILLDEX_RC_STEAL = static_cast<int>(SkilldexRC::Steal);
inline constexpr int SKILLDEX_RC_TRAPS = static_cast<int>(SkilldexRC::Traps);
inline constexpr int SKILLDEX_RC_FIRST_AID = static_cast<int>(SkilldexRC::FirstAid);
inline constexpr int SKILLDEX_RC_DOCTOR = static_cast<int>(SkilldexRC::Doctor);
inline constexpr int SKILLDEX_RC_SCIENCE = static_cast<int>(SkilldexRC::Science);
inline constexpr int SKILLDEX_RC_REPAIR = static_cast<int>(SkilldexRC::Repair);

int skilldex_select();

} // namespace fallout
