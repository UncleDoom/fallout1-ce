#pragma once

// Semantic type aliases for the Fallout engine.
//
// These replace the raw `int` soup in Object, Proto, MapHeader, etc.
// with self-documenting names. They are type aliases (not strong types)
// so they are source-compatible with existing code — no casts required.
//
// Future: consider converting these to strong types (e.g., via a
// tagged-int template) once all call sites are migrated.

#include <cstdint>

namespace fallout {

/// Unique object identifier (obj_id). -1 = invalid.
using ObjectId = int;

/// Hex tile index in the map grid. -1 = invalid/off-map.
using TileIndex = int;

/// Map elevation (0, 1, or 2).
using Elevation = int;

/// Frame ID — encodes art type, item/critter ID, and animation.
/// Use FID_TYPE() to extract the object type from the top byte.
using Fid = int;

/// Prototype ID — encodes object type in the top byte.
/// Use PID_TYPE() to extract the object type.
using Pid = int;

/// Script ID — encodes script type in the top byte.
/// Use SID_TYPE() to extract the script type.
using Sid = int;

/// Combat ID — temporary identifier during combat sequences.
using CombatId = int;

/// Index into the inventory items array.
using InventoryIndex = int;

/// Light intensity value (0–65536 range typically).
using LightIntensity = int;

/// Light distance in hex tiles.
using LightDistance = int;

/// Animation frame index.
using FrameIndex = int;

// Sentinel values
inline constexpr ObjectId INVALID_OBJECT_ID = -1;
inline constexpr TileIndex INVALID_TILE = -1;
inline constexpr Fid INVALID_FID = -1;
inline constexpr Pid INVALID_PID = -1;
inline constexpr Sid INVALID_SID = -1;

} // namespace fallout
