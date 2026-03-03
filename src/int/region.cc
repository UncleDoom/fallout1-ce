#include "int/region.h"

#include <climits>
#include <cstring>

#include "plib/gnw/debug.h"

namespace fallout {

// --- Region class implementation ---

Region::Region(int initialCapacity)
{
    if (initialCapacity > 0) {
        points_.reserve(initialCapacity + 1);
    }
    minX_ = INT_MIN;
    minY_ = INT_MIN;
    maxX_ = INT_MAX;
    maxY_ = INT_MAX;
}

void Region::setBound()
{
    int minX = INT_MAX;
    int maxX = INT_MIN;
    int minY = INT_MAX;
    int maxY = INT_MIN;
    int totalX = 0;
    int totalY = 0;
    int numPoints = 0;

    // Last point is the duplicate closing point; iterate only real points.
    const int count = points_.empty() ? 0 : static_cast<int>(points_.size()) - 1;
    for (int i = 0; i < count; i++) {
        const Point& pt = points_[i];
        if (minX >= pt.x) minX = pt.x;
        if (minY >= pt.y) minY = pt.y;
        if (maxX <= pt.x) maxX = pt.x;
        if (maxY <= pt.y) maxY = pt.y;
        totalX += pt.x;
        totalY += pt.y;
        numPoints++;
    }

    minY_ = minY;
    maxX_ = maxX;
    maxY_ = maxY;
    minX_ = minX;

    if (numPoints != 0) {
        centerX_ = totalX / numPoints;
        centerY_ = totalY / numPoints;
    }
}

bool Region::contains(int x, int y) const
{
    if (x < minX_ || x > maxX_ || y < minY_ || y > maxY_) {
        return false;
    }

    const int count = points_.empty() ? 0 : static_cast<int>(points_.size()) - 1;
    if (count <= 0) {
        return false;
    }

    int v1;
    const Point& first = points_[0];
    if (x >= first.x) {
        v1 = (y >= first.y) ? 2 : 1;
    } else {
        v1 = (y >= first.y) ? 3 : 0;
    }

    int v4 = 0;
    const Point* prev = &points_[0];
    for (int index = 0; index < count; index++) {
        const Point* point = &points_[index + 1];
        int v2;
        if (x >= point->x) {
            v2 = (y >= point->y) ? 2 : 1;
        } else {
            v2 = (y >= point->y) ? 3 : 0;
        }

        int v3 = v2 - v1;
        switch (v3) {
        case -3:
            v3 = 1;
            break;
        case -2:
        case 2:
            if (static_cast<double>(x) < (static_cast<double>(point->x) - static_cast<double>(prev->x - point->x) / static_cast<double>(prev->y - point->y) * static_cast<double>(point->y - y))) {
                v3 = -v3;
            }
            break;
        case 3:
            v3 = -1;
            break;
        }

        prev = point;
        v1 = v2;
        v4 += v3;
    }

    return (v4 == 4 || v4 == -4);
}

void Region::addPoint(int x, int y)
{
    if (points_.empty()) {
        // First point: add the point and a closing duplicate.
        points_.push_back({ x, y });
        points_.push_back({ x, y });
    } else {
        // Insert before the closing duplicate point.
        const int pointIndex = static_cast<int>(points_.size()) - 1;
        points_.insert(points_.begin() + pointIndex, { x, y });
        // Update closing point to match the first point.
        points_.back() = points_.front();
    }
}

void Region::setName(const char* name)
{
    if (name == nullptr) {
        name_[0] = '\0';
        return;
    }
    strncpy(name_, name, REGION_NAME_LENGTH - 1);
    name_[REGION_NAME_LENGTH - 1] = '\0';
}

// --- Legacy free-function wrappers ---

void regionSetBound(Region* region)
{
    if (region != nullptr) {
        region->setBound();
    }
}

bool pointInRegion(Region* region, int x, int y)
{
    if (region == nullptr) {
        return false;
    }
    return region->contains(x, y);
}

Region* allocateRegion(int initialCapacity)
{
    return new Region(initialCapacity);
}

void regionAddPoint(Region* region, int x, int y)
{
    if (region == nullptr) {
        debug_printf("regionAddPoint(): null region ptr\n");
        return;
    }
    region->addPoint(x, y);
}

void regionDelete(Region* region)
{
    if (region == nullptr) {
        debug_printf("regionDelete(): null region ptr\n");
        return;
    }
    delete region;
}

void regionAddName(Region* region, const char* name)
{
    if (region == nullptr) {
        debug_printf("regionAddName(): null region ptr\n");
        return;
    }
    region->setName(name);
}

const char* regionGetName(Region* region)
{
    if (region == nullptr) {
        debug_printf("regionGetName(): null region ptr\n");
        return "<null>";
    }
    return region->getName();
}

void* regionGetUserData(Region* region)
{
    if (region == nullptr) {
        debug_printf("regionGetUserData(): null region ptr\n");
        return nullptr;
    }
    return region->getUserData();
}

void regionSetUserData(Region* region, void* data)
{
    if (region == nullptr) {
        debug_printf("regionSetUserData(): null region ptr\n");
        return;
    }
    region->setUserData(data);
}

void regionSetFlag(Region* region, int value)
{
    if (region != nullptr) {
        region->setFlag(value);
    }
}

int regionGetFlag(Region* region)
{
    if (region != nullptr) {
        return region->getFlags();
    }
    return 0;
}

} // namespace fallout
