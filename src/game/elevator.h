#pragma once


namespace fallout {

enum class Elevator : int {
    BrotherhoodOfSteelMain = 0,
    BrotherhoodOfSteelSurface = 1,
    MasterUpper = 2,
    MasterLower = 3,
    MilitaryBaseUpper = 4,
    MilitaryBaseLower = 5,
    GlowUpper = 6,
    GlowLower = 7,
    Vault13 = 8,
    Necropolis = 9,
    Sierra1 = 10,
    Sierra2 = 11,
    Count = 12,
};

inline constexpr int ELEVATOR_BROTHERHOOD_OF_STEEL_MAIN = static_cast<int>(Elevator::BrotherhoodOfSteelMain);
inline constexpr int ELEVATOR_BROTHERHOOD_OF_STEEL_SURFACE = static_cast<int>(Elevator::BrotherhoodOfSteelSurface);
inline constexpr int ELEVATOR_MASTER_UPPER = static_cast<int>(Elevator::MasterUpper);
inline constexpr int ELEVATOR_MASTER_LOWER = static_cast<int>(Elevator::MasterLower);
inline constexpr int ELEVATOR_MILITARY_BASE_UPPER = static_cast<int>(Elevator::MilitaryBaseUpper);
inline constexpr int ELEVATOR_MILITARY_BASE_LOWER = static_cast<int>(Elevator::MilitaryBaseLower);
inline constexpr int ELEVATOR_GLOW_UPPER = static_cast<int>(Elevator::GlowUpper);
inline constexpr int ELEVATOR_GLOW_LOWER = static_cast<int>(Elevator::GlowLower);
inline constexpr int ELEVATOR_VAULT_13 = static_cast<int>(Elevator::Vault13);
inline constexpr int ELEVATOR_NECROPOLIS = static_cast<int>(Elevator::Necropolis);
inline constexpr int ELEVATOR_SIERRA_1 = static_cast<int>(Elevator::Sierra1);
inline constexpr int ELEVATOR_SIERRA_2 = static_cast<int>(Elevator::Sierra2);
inline constexpr int ELEVATOR_COUNT = static_cast<int>(Elevator::Count);

int elevator_select(int elevator, int* mapPtr, int* elevationPtr, int* tilePtr);

} // namespace fallout
