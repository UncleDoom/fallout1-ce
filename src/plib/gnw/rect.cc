#include "plib/gnw/rect.h"

#include <cstdlib>

#include <algorithm>

#include "plib/gnw/memory.h"

namespace fallout {

// 0x539D58
static RectPtr rlist = nullptr;

// 0x4B29B0
void GNW_rect_exit()
{
    RectPtr temp;

    while (rlist != nullptr) {
        temp = rlist->next;
        mem_free(rlist);
        rlist = temp;
    }
}

// 0x4B29D4
void rect_clip_list(RectPtr* pCur, Rect* bound)
{
    Rect v1 = *bound;

    // NOTE: Original code is slightly different.
    while (*pCur != nullptr) {
        RectPtr rectListNode = *pCur;
        if (v1.lrx >= rectListNode->rect.ulx
            && v1.lry >= rectListNode->rect.uly
            && v1.ulx <= rectListNode->rect.lrx
            && v1.uly <= rectListNode->rect.lry) {
            Rect v2 = rectListNode->rect;

            *pCur = rectListNode->next;

            rectListNode->next = rlist;
            rlist = rectListNode;

            if (v2.uly < v1.uly) {
                RectPtr newRectListNode = rect_malloc();
                if (newRectListNode == nullptr) {
                    return;
                }

                newRectListNode->rect = v2;
                newRectListNode->rect.lry = v1.uly - 1;
                newRectListNode->next = *pCur;

                *pCur = newRectListNode;
                pCur = &(newRectListNode->next);

                v2.uly = v1.uly;
            }

            if (v2.lry > v1.lry) {
                RectPtr newRectListNode = rect_malloc();
                if (newRectListNode == nullptr) {
                    return;
                }

                newRectListNode->rect = v2;
                newRectListNode->rect.uly = v1.lry + 1;
                newRectListNode->next = *pCur;

                *pCur = newRectListNode;
                pCur = &(newRectListNode->next);

                v2.lry = v1.lry;
            }

            if (v2.ulx < v1.ulx) {
                RectPtr newRectListNode = rect_malloc();
                if (newRectListNode == nullptr) {
                    return;
                }

                newRectListNode->rect = v2;
                newRectListNode->rect.lrx = v1.ulx - 1;
                newRectListNode->next = *pCur;

                *pCur = newRectListNode;
                pCur = &(newRectListNode->next);
            }

            if (v2.lrx > v1.lrx) {
                RectPtr newRectListNode = rect_malloc();
                if (newRectListNode == nullptr) {
                    return;
                }

                newRectListNode->rect = v2;
                newRectListNode->rect.ulx = v1.lrx + 1;
                newRectListNode->next = *pCur;

                *pCur = newRectListNode;
                pCur = &(newRectListNode->next);
            }
        } else {
            pCur = &(rectListNode->next);
        }
    }
}

// 0x4B2B5C
RectPtr rect_clip(Rect* b, Rect* t)
{
    Rect clipped_t;
    RectPtr list;
    RectPtr* next;
    Rect clipped_b[4];
    int k;

    list = nullptr;

    if (t->insideBound(*b, clipped_t) == 0) {
        clipped_b[0].ulx = b->ulx;
        clipped_b[0].uly = b->uly;
        clipped_b[0].lrx = b->lrx;
        clipped_b[0].lry = clipped_t.uly - 1;

        clipped_b[1].ulx = b->ulx;
        clipped_b[1].uly = clipped_t.uly;
        clipped_b[1].lrx = clipped_t.ulx - 1;
        clipped_b[1].lry = clipped_t.lry;

        clipped_b[2].ulx = clipped_t.lrx + 1;
        clipped_b[2].uly = clipped_t.uly;
        clipped_b[2].lrx = b->lrx;
        clipped_b[2].lry = clipped_t.lry;

        clipped_b[3].ulx = b->ulx;
        clipped_b[3].uly = clipped_t.lry + 1;
        clipped_b[3].lrx = b->lrx;
        clipped_b[3].lry = b->lry;

        next = &list;
        for (k = 0; k < 4; k++) {
            if (clipped_b[k].lrx >= clipped_b[k].ulx && clipped_b[k].lry >= clipped_b[k].uly) {
                list = rect_malloc();
                *next = list;
                if (list == nullptr) {
                    return nullptr;
                }

                list->rect = clipped_b[k];
                list->next = nullptr;

                next = &list;
            }
        }
    } else {
        list = rect_malloc();
        if (list != nullptr) {
            list->rect.ulx = b->ulx;
            list->rect.uly = b->uly;
            list->rect.lrx = b->lrx;
            list->rect.lry = b->lry;
            list->next = nullptr;
        }
    }

    return list;
}

// 0x4B2C68
RectPtr rect_malloc()
{
    RectPtr temp;
    int i;

    if (rlist == nullptr) {
        for (i = 0; i < 10; i++) {
            temp = static_cast<RectPtr>(mem_malloc(sizeof(*temp)));
            if (temp == nullptr) {
                break;
            }

            temp->next = rlist;
            rlist = temp;
        }
    }

    if (rlist == nullptr) {
        return nullptr;
    }

    temp = rlist;
    rlist = rlist->next;

    return temp;
}

// 0x4B2CB4
void rect_free(RectPtr ptr)
{
    ptr->next = rlist;
    rlist = ptr;
}

// 0x4B2D18
int Rect::insideBound(const Rect& bound, Rect& result) const noexcept
{
    result.ulx = ulx;
    result.uly = uly;
    result.lrx = lrx;
    result.lry = lry;

    if (ulx <= bound.lrx && bound.ulx <= lrx && bound.lry >= uly && bound.uly <= lry) {
        if (bound.ulx > ulx) {
            result.ulx = bound.ulx;
        }

        if (bound.lrx < lrx) {
            result.lrx = bound.lrx;
        }

        if (bound.uly > uly) {
            result.uly = bound.uly;
        }

        if (bound.lry < lry) {
            result.lry = bound.lry;
        }

        return 0;
    }

    return -1;
}



} // namespace fallout
