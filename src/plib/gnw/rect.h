#pragma once

#include <algorithm>

namespace fallout {

struct Point {
    int x;
    int y;

    constexpr Point() noexcept : x(0), y(0) {}
    constexpr Point(int x, int y) noexcept : x(x), y(y) {}

    constexpr bool operator==(const Point& other) const noexcept { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Point& other) const noexcept { return !(*this == other); }

    constexpr Point offset(int dx, int dy) const noexcept { return { x + dx, y + dy }; }
};

struct Size {
    int width;
    int height;

    constexpr Size() noexcept : width(0), height(0) {}
    constexpr Size(int w, int h) noexcept : width(w), height(h) {}

    [[nodiscard]] constexpr int area() const noexcept { return width * height; }
    [[nodiscard]] constexpr bool empty() const noexcept { return width <= 0 || height <= 0; }

    constexpr bool operator==(const Size& other) const noexcept { return width == other.width && height == other.height; }
    constexpr bool operator!=(const Size& other) const noexcept { return !(*this == other); }
};

struct Rect {
    int ulx; // upper-left x
    int uly; // upper-left y
    int lrx; // lower-right x
    int lry; // lower-right y

    constexpr Rect() noexcept : ulx(0), uly(0), lrx(0), lry(0) {}
    constexpr Rect(int ulx, int uly, int lrx, int lry) noexcept
        : ulx(ulx), uly(uly), lrx(lrx), lry(lry) {}

    [[nodiscard]] constexpr int width() const noexcept { return lrx - ulx + 1; }
    [[nodiscard]] constexpr int height() const noexcept { return lry - uly + 1; }
    [[nodiscard]] constexpr Size size() const noexcept { return { width(), height() }; }
    [[nodiscard]] constexpr bool empty() const noexcept { return lrx < ulx || lry < uly; }

    [[nodiscard]] constexpr bool contains(int x, int y) const noexcept
    {
        return x >= ulx && x <= lrx && y >= uly && y <= lry;
    }

    [[nodiscard]] constexpr bool contains(const Point& p) const noexcept
    {
        return contains(p.x, p.y);
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept
    {
        return ulx <= other.lrx && lrx >= other.ulx
            && uly <= other.lry && lry >= other.uly;
    }

    /// Returns the intersection of this rect with `other`, or an empty rect if disjoint.
    [[nodiscard]] Rect intersection(const Rect& other) const noexcept
    {
        return {
            std::max(ulx, other.ulx),
            std::max(uly, other.uly),
            std::min(lrx, other.lrx),
            std::min(lry, other.lry),
        };
    }

    /// Returns the minimal bounding rect that contains both this and `other`.
    [[nodiscard]] Rect united(const Rect& other) const noexcept
    {
        return {
            std::min(ulx, other.ulx),
            std::min(uly, other.uly),
            std::max(lrx, other.lrx),
            std::max(lry, other.lry),
        };
    }

    void offset(int dx, int dy) noexcept
    {
        ulx += dx;
        uly += dy;
        lrx += dx;
        lry += dy;
    }

    [[nodiscard]] Rect offsetted(int dx, int dy) const noexcept
    {
        return { ulx + dx, uly + dy, lrx + dx, lry + dy };
    }

    /// Expands this rect to also contain `other` (union in place).
    void minBound(const Rect& other) noexcept
    {
        ulx = std::min(ulx, other.ulx);
        uly = std::min(uly, other.uly);
        lrx = std::max(lrx, other.lrx);
        lry = std::max(lry, other.lry);
    }

    /// Clips `this` to `bound`, storing the result in `result`.
    /// Returns 0 if the rects intersect, -1 otherwise (result is a copy of *this).
    int insideBound(const Rect& bound, Rect& result) const noexcept;
};

// Linked-list node for rect clipping operations.
struct rectdata {
    Rect rect;
    struct rectdata* next;
};

using RectPtr = rectdata*;

void GNW_rect_exit();
void rect_clip_list(RectPtr* pCur, Rect* bound);
RectPtr rect_clip(Rect* b, Rect* t);
RectPtr rect_malloc();
void rect_free(RectPtr ptr);

} // namespace fallout
