/*
 * mm.c - Implicit free list with boundary tags.
 *
 * Ordinary blocks have a 4-byte header, an 8-byte-aligned payload, and a
 * 4-byte footer. Both tags store the total block size and allocation bit.
 * Allocation uses first fit and splitting; freeing immediately coalesces
 * free neighbors. Allocated prologue and epilogue tags bound the heap.
 */
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Zunmi",
    /* First member's full name */
    "Zunmi",
    /* First member's email address */
    "13029579578@163.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define MIN_BLOCK_SIZE (2 * DSIZE)
#define CHUNKSIZE (1 << 12)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define PACK(size, alloc) ((size) | (alloc))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static char *heap_listp;

static void *coalesce(void *bp);
static void *extend_heap(size_t size);
static void *find_fit(size_t size);
static void place(void *bp, size_t size);

/* Build the aligned prologue, epilogue, and initial free block. */
int mm_init(void)
{
    heap_listp = mem_sbrk(4 * WSIZE);
    if (heap_listp == (void *)-1) {
        heap_listp = NULL;
        return -1;
    }

    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1));
    heap_listp += DSIZE;

    if (extend_heap(CHUNKSIZE) == NULL)
        return -1;
    return 0;
}

/* Find the first fitting block, extending the heap only when necessary. */
void *mm_malloc(size_t size)
{
    size_t block_size;
    size_t extend_size;
    void *bp;

    /* mem_sbrk accepts int; reject overflow before adding tags and padding. */
    if (size == 0 || size > INT_MAX - DSIZE - (ALIGNMENT - 1))
        return NULL;

    block_size = ALIGN(size + DSIZE);

    bp = find_fit(block_size);
    if (bp == NULL) {
        extend_size = block_size > CHUNKSIZE ? block_size : CHUNKSIZE;
        bp = extend_heap(extend_size);
        if (bp == NULL)
            return NULL;
    }

    place(bp, block_size);
    return bp;
}

/* Mark both tags free and merge with any free physical neighbors. */
void mm_free(void *ptr)
{
    size_t size;

    if (ptr == NULL)
        return;

    size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/* Preserve the old allocation until a replacement has been obtained. */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    size_t copy_size;

    if (ptr == NULL)
        return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    /* shortcut: always allocate and copy; add in-place resizing after the basic version is understood. */
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    copy_size = GET_SIZE(HDRP(ptr)) - DSIZE;
    if (size < copy_size)
        copy_size = size;
    memcpy(newptr, ptr, copy_size);
    mm_free(ptr);
    return newptr;
}

/* Merge the free block with its left and right neighbors independently. */
static void *coalesce(void *bp)
{
    void *prev = PREV_BLKP(bp);
    void *next = NEXT_BLKP(bp);
    size_t size = GET_SIZE(HDRP(bp));

    if (!GET_ALLOC(HDRP(prev))) {
        size += GET_SIZE(HDRP(prev));
        bp = prev;
    }
    if (!GET_ALLOC(HDRP(next)))
        size += GET_SIZE(HDRP(next));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    return bp;
}

/* Reuse the old epilogue as a free header and append a new epilogue. */
static void *extend_heap(size_t size)
{
    char *bp = mem_sbrk((int)size);

    if (bp == (void *)-1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

/* Walk all blocks by size until the first large enough free block. */
static void *find_fit(size_t size)
{
    void *bp;

    for (bp = NEXT_BLKP(heap_listp); GET_SIZE(HDRP(bp)) != 0;
         bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= size)
            return bp;
    }
    return NULL;
}

/* Split only when the remainder can form a complete minimum-size block. */
static void place(void *bp, size_t size)
{
    size_t block_size = GET_SIZE(HDRP(bp));

    if (block_size - size >= MIN_BLOCK_SIZE) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(block_size - size, 0));
        PUT(FTRP(bp), PACK(block_size - size, 0));
    } else {
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
}

/* Check alignment, bounds, matching tags, coalescing, and heap sentinels. */
int mm_check(void)
{
    char *bp = heap_listp;
    char *heap_end = (char *)mem_heap_lo() + mem_heapsize();
    int previous_free = 0;

    if (bp == NULL || GET(HDRP(bp)) != PACK(DSIZE, 1) ||
        GET(FTRP(bp)) != PACK(DSIZE, 1))
        return 0;

    for (bp = NEXT_BLKP(bp); ; bp = NEXT_BLKP(bp)) {
        size_t size;
        int allocated;

        if ((uintptr_t)bp % ALIGNMENT != 0 || bp > heap_end)
            return 0;
        size = GET_SIZE(HDRP(bp));
        allocated = GET_ALLOC(HDRP(bp));
        if (size == 0)
            return allocated && bp == heap_end;

        if (size < MIN_BLOCK_SIZE || size > (size_t)(heap_end - bp) ||
            GET(HDRP(bp)) != GET(FTRP(bp)) ||
            (!allocated && previous_free))
            return 0;
        previous_free = !allocated;
    }
}
