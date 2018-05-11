/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <trylock.h>
//#include <atomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <heap.h>
//#include <seos.h>

#if 0
#define TIDX_HEAP_EXTRA 2 // must be >= 0; best if > 0, don't make it > 7, since it unnecessarily limits max heap size we can manage

#define TIDX_HEAP_BITS (TASK_IDX_BITS + TIDX_HEAP_EXTRA)

#define TIDX_MASK ((1 << TIDX_HEAP_BITS) - 1)
#define MAX_HEAP_ORDER (31 - TIDX_HEAP_BITS)

#if MAX_HEAP_ORDER < 16
# error Too little HEAP is available
#endif

struct HeapNode {

    struct HeapNode* prev;
    uint32_t size: MAX_HEAP_ORDER;
    uint32_t used: 1;
    uint32_t tidx: TIDX_HEAP_BITS; // TASK_IDX_BITS to uniquely identify task; + extra bits of redundant counter add extra protection
    uint8_t  data[];
};

#ifdef FORCE_HEAP_IN_DOT_DATA

    static uint8_t __attribute__ ((aligned (8))) gHeap[HEAP_SIZE];

    #define REAL_HEAP_SIZE     ((HEAP_SIZE) &~ 7)
    #define ALIGNED_HEAP_START (&gHeap)

#else

    extern uint8_t __heap_end[], __heap_start[];
    #define ALIGNED_HEAP_START  (uint8_t*)((((uintptr_t)&__heap_start) + 7) &~ 7)
    #define ALIGNED_HEAP_END    (uint8_t*)(((uintptr_t)&__heap_end) &~ 7)

    #define REAL_HEAP_SIZE      (ALIGNED_HEAP_END - ALIGNED_HEAP_START)


#endif
#endif

#if 0
static struct HeapNode* gHeapHead;
static TRYLOCK_DECL_STATIC(gHeapLock) = TRYLOCK_INIT_STATIC();
static volatile uint8_t gNeedFreeMerge = false; /* cannot be bool since its size is ill defined */
static struct HeapNode *gHeapTail;
#endif

#if 0
static inline struct HeapNode* heapPrvGetNext(struct HeapNode* node)
{
    return (gHeapTail == node) ? NULL : (struct HeapNode*)(node->data + node->size);
}
#endif

#if 0
//called to merge free chunks in case free() was unable to last time it tried. only call with lock held please
static void heapMergeFreeChunks(void)
{
    while (atomicXchgByte(&gNeedFreeMerge, false)) {
        struct HeapNode *node = gHeapHead, *next;

        while (node) {
            next = heapPrvGetNext(node);

            if (!node->used && next && !next->used) { /* merged */
                node->size += sizeof(struct HeapNode) + next->size;

                next = heapPrvGetNext(node);
                if (next)
                    next->prev = node;
                else
                    gHeapTail = node;
            }
            else
                node = next;
        }
    }
}
#endif

void* heapAlloc(uint32_t sz) {
  void* ret = NULL;

  ret = malloc((size_t)sz);
  return ret;
}

void heapFree(void* ptr) {
  if (ptr == NULL)
    return;
  free(ptr);
}

#if 0
int heapFreeAll(uint32_t tid)
{
    struct HeapNode *node;
    bool haveLock;
    int count = 0;

    if (!tid)
        return -1;

    // this can only fail if called from interrupt
    haveLock = trylockTryTake(&gHeapLock);
    if (!haveLock)
        return -1;

    node = gHeapHead;
    tid &= TIDX_MASK;
    do {
        if (node->tidx == tid) {
            node->used = 0;
            node->tidx = 0;
            count++;
        }
    } while ((node = heapPrvGetNext(node)) != NULL);
    gNeedFreeMerge = true;
    trylockRelease(&gHeapLock);

    return count;
}
#endif
