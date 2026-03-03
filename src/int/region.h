#pragma once

#include <cstring>
#include <vector>

#include "int/intrpret.h"
#include "plib/gnw/rect.h"

namespace fallout {

inline constexpr int REGION_NAME_LENGTH = 32;

class Region;

using RegionMouseEventCallback = void(Region* region, void* userData, int event);

/// 2D polygonal region for point-in-region testing.
/// Replaces the former C-style struct + free-function pattern.
class Region {
public:
    /// Create a region with an optional initial point capacity.
    explicit Region(int initialCapacity = 0);
    ~Region() = default;

    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;

    /// Add a vertex to the polygon.
    void addPoint(int x, int y);

    /// Recompute the bounding box and centroid from current points.
    void setBound();

    /// Test whether a point lies inside this polygonal region.
    [[nodiscard]] bool contains(int x, int y) const;

    /// Access a vertex by index.
    [[nodiscard]] const Point& getPoint(int index) const { return points_[index]; }

    /// Set a human-readable name for this region.
    void setName(const char* name);

    /// Get the region's name.
    [[nodiscard]] const char* getName() const noexcept { return name_; }

    /// Get/set opaque user data pointer.
    [[nodiscard]] void* getUserData() const noexcept { return userData_; }
    void setUserData(void* data) noexcept { userData_ = data; }

    /// Get/set flag bitmask.
    [[nodiscard]] int getFlags() const noexcept { return flags_; }
    void setFlag(int value) noexcept { flags_ |= value; }

    // Public members that are directly accessed by the scripting engine.
    // These remain public for backward compatibility during migration.
    Program* program = nullptr;
    int procs[4] = {};
    int rightProcs[4] = {};
    int field_68 = 0;
    int field_6C = 0;
    int field_70 = 0;
    RegionMouseEventCallback* mouseEventCallback = nullptr;
    RegionMouseEventCallback* rightMouseEventCallback = nullptr;
    void* mouseEventCallbackUserData = nullptr;
    void* rightMouseEventCallbackUserData = nullptr;

private:
    char name_[REGION_NAME_LENGTH] = {};
    std::vector<Point> points_;
    int minX_ = 0;
    int minY_ = 0;
    int maxX_ = 0;
    int maxY_ = 0;
    int centerX_ = 0;
    int centerY_ = 0;
    int flags_ = 0;
    void* userData_ = nullptr;
};

// Legacy free-function wrappers — prefer Region member methods in new code.
void regionSetBound(Region* region);
bool pointInRegion(Region* region, int x, int y);
Region* allocateRegion(int initialCapacity);
void regionAddPoint(Region* region, int x, int y);
void regionDelete(Region* region);
void regionAddName(Region* region, const char* src);
const char* regionGetName(Region* region);
void* regionGetUserData(Region* region);
void regionSetUserData(Region* region, void* data);
void regionSetFlag(Region* region, int value);
int regionGetFlag(Region* region);

} // namespace fallout
