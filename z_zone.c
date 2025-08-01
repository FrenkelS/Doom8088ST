// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2023-2025 by Frenkel Smeijers
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Zone Memory Allocation. Neat.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include "compiler.h"
#include "z_zone.h"
#include "doomdef.h"
#include "i_system.h"


//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC		1	// static entire execution time
#define PU_LEVEL		2	// static until level exited
#define PU_LEVSPEC		3	// a special thinker in a level
#define PU_CACHE		4

#define PU_PURGELEVEL PU_CACHE


//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

#if defined INSTRUMENTED
    static int32_t running_count = 0;
#endif


#define	ZONEID	0x1dea

typedef struct
{
#if SIZE_OF_SEGMENT_T == 2
    uint32_t  size;			// including the header and possibly tiny fragments
    uint16_t  tag;			// purgelevel
#else
    uint32_t  size:24;		// including the header and possibly tiny fragments
    uint32_t  tag:4;		// purgelevel
#endif
    void __far*__far*    user;	// NULL if a free block
    segment_t next;
    segment_t prev;
#if defined ZONEIDCHECK
    uint16_t id;			// should be ZONEID
#endif
} memblock_t;


typedef char assertMemblockSize[sizeof(memblock_t) <= PARAGRAPH_SIZE ? 1 : -1];


static memblock_t __far* mainzone_sentinal;
static segment_t   mainzone_rover_segment;


static segment_t pointerToSegment(const memblock_t __far* ptr)
{
#if defined RANGECHECK
	if ((((uint32_t) ptr) & (PARAGRAPH_SIZE - 1)) != 0)
		I_Error("pointerToSegment: pointer is not aligned: 0x%lx", ptr);
#endif

	return D_FP_SEG(ptr);
}

static memblock_t __far* segmentToPointer(segment_t seg)
{
	return D_MK_FP(seg, 0);
}


boolean Z_EqualNames(const char __far* farName, const char* nearName)
{
	return *(uint64_t __far*)farName == *(uint64_t*)nearName;
}


//
// Z_Init
//
void Z_Init (void)
{
	// allocate all available conventional memory.
	uint32_t heapSize;
	static uint8_t __far* mainzone; mainzone = I_ZoneBase(&heapSize);

	printf("%ld bytes allocated for zone\r\n", heapSize);

	// align blocklist
	uint_fast8_t i = 0;
	static uint8_t __far mainzone_sentinal_buffer[PARAGRAPH_SIZE * 2];
	uint32_t b = (uint32_t) &mainzone_sentinal_buffer[i++];
	while ((b & (PARAGRAPH_SIZE - 1)) != 0)
		b = (uint32_t) &mainzone_sentinal_buffer[i++];
	mainzone_sentinal = (memblock_t __far*)b;

#if defined __WATCOMC__ && defined _M_I86
	// normalize pointer
	mainzone_sentinal = D_MK_FP(D_FP_SEG(mainzone_sentinal) + D_FP_OFF(mainzone_sentinal) / PARAGRAPH_SIZE, 0);
#endif

	// set the entire zone to one free block
	memblock_t __far* block = (memblock_t __far*)mainzone;
	mainzone_rover_segment = pointerToSegment(block);

	mainzone_sentinal->tag  = PU_STATIC;
	mainzone_sentinal->user = (void __far*)mainzone;
	mainzone_sentinal->next = mainzone_rover_segment;
	mainzone_sentinal->prev = mainzone_rover_segment;

	block->size = heapSize;
	block->tag  = 0;
	block->user = NULL; // NULL indicates a free block.
	block->prev = pointerToSegment(mainzone_sentinal);
	block->next = block->prev;
#if defined ZONEIDCHECK
	block->id   = ZONEID;
#endif
}


static void Z_ChangeTag(const void __far* ptr, uint_fast8_t tag)
{
#if defined RANGECHECK
	if ((((uint32_t) ptr) & (PARAGRAPH_SIZE - 1)) != 0)
		I_Error("Z_ChangeTag: pointer is not aligned: 0x%lx", ptr);
#endif

#if defined _M_I86
	memblock_t __far* block = (memblock_t __far*)(((uint32_t)ptr) - 0x00010000);
#else
	memblock_t __far* block = (memblock_t __far*)(((uint32_t)ptr) - 0x00010);
#endif

#if defined ZONEIDCHECK
	if (block->id != ZONEID)
		I_Error("Z_ChangeTag: block has id %x instead of ZONEID", block->id);
#endif
	block->tag = tag;
}


void Z_ChangeTagToStatic(const void __far* ptr)
{
	Z_ChangeTag(ptr, PU_STATIC);
}


void Z_ChangeTagToCache(const void __far* ptr)
{
	Z_ChangeTag(ptr, PU_CACHE);
}


static void Z_FreeBlock(memblock_t __far* block)
{
#if defined ZONEIDCHECK
    if (block->id != ZONEID)
        I_Error("Z_FreeBlock: block has id %x instead of ZONEID", block->id);
#endif

    if (D_FP_SEG(block->user) != 0)
    {
        // far pointers with segment 0 are not user pointers
        // Note: OS-dependend

        // clear the user's mark
        *block->user = NULL;
    }

    // mark as free
    block->user = NULL;
    block->tag  = 0;


#if defined INSTRUMENTED
    running_count -= block->size;
    printf("Free: %ld\r\n", running_count);
#endif

    memblock_t __far* other = segmentToPointer(block->prev);

    if (!other->user)
    {
        // merge with previous free block
        other->size += block->size;
        other->next  = block->next;
        segmentToPointer(other->next)->prev = block->prev; // == pointerToSegment(other);

        if (pointerToSegment(block) == mainzone_rover_segment)
            mainzone_rover_segment = block->prev; // == pointerToSegment(other);

        block = other;
    }

    other = segmentToPointer(block->next);
    if (!other->user)
    {
        // merge the next free block onto the end
        block->size += other->size;
        block->next  = other->next;
        segmentToPointer(block->next)->prev = pointerToSegment(block);

        if (pointerToSegment(other) == mainzone_rover_segment)
            mainzone_rover_segment = pointerToSegment(block);
    }
}


//
// Z_Free
//
void Z_Free(const void __far* ptr)
{
#if defined RANGECHECK
	if ((((uint32_t) ptr) & (PARAGRAPH_SIZE - 1)) != 0)
		I_Error("Z_Free: pointer is not aligned: 0x%lx", ptr);
#endif

#if defined _M_I86
	memblock_t __far* block = (memblock_t __far*)(((uint32_t)ptr) - 0x00010000);
#else
	memblock_t __far* block = (memblock_t __far*)(((uint32_t)ptr) - 0x00010);
#endif

	Z_FreeBlock(block);
}


static uint32_t Z_GetLargestFreeBlockSize(void)
{
	uint32_t largestFreeBlockSize = 0;

	segment_t mainzone_sentinal_segment = pointerToSegment(mainzone_sentinal);

	for (memblock_t __far* block = segmentToPointer(mainzone_sentinal->next); pointerToSegment(block) != mainzone_sentinal_segment; block = segmentToPointer(block->next))
		if (!block->user && block->size > largestFreeBlockSize)
			largestFreeBlockSize = block->size;

	return largestFreeBlockSize;
}

static uint32_t Z_GetTotalFreeMemory(void)
{
	uint32_t totalFreeMemory = 0;

	segment_t mainzone_sentinal_segment = pointerToSegment(mainzone_sentinal);

	for (memblock_t __far* block = segmentToPointer(mainzone_sentinal->next); pointerToSegment(block) != mainzone_sentinal_segment; block = segmentToPointer(block->next))
		if (!block->user)
			totalFreeMemory += block->size;

	return totalFreeMemory;
}


//
// Z_TryMalloc
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
// Because Z_TryMalloc is static, we can control the input and we can make sure tag is always < PU_PURGELEVEL.
//
#define MINFRAGMENT		64


static void __far* Z_TryMalloc(uint16_t size, int8_t tag, void __far*__far* user)
{
    size = (size + (PARAGRAPH_SIZE - 1)) & ~(PARAGRAPH_SIZE - 1);

    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += PARAGRAPH_SIZE;

    // if there is a free block behind the rover,
    //  back up over them
    memblock_t __far* base = segmentToPointer(mainzone_rover_segment);

    memblock_t __far* previous_block = segmentToPointer(base->prev);
    if (!previous_block->user)
        base = previous_block;

    memblock_t __far* rover   = base;
    segment_t   start_segment = base->prev;

    do
    {
        if (pointerToSegment(rover) == start_segment)
        {
            // scanned all the way around the list
            return NULL;
        }

        if (rover->user)
        {
            if (rover->tag < PU_PURGELEVEL)
            {
                // hit a block that can't be purged,
                //  so move base past it
                base = rover = segmentToPointer(rover->next);
            }
            else
            {
                // free the rover block (adding the size to base)

                // the rover can be the base block
                base  = segmentToPointer(base->prev);
                Z_FreeBlock(rover);
                base  = segmentToPointer(base->next);
                rover = segmentToPointer(base->next);
            }
        }
        else
            rover = segmentToPointer(rover->next);

    } while (base->user || base->size < size);
    // found a block big enough

    int32_t newblock_size = base->size - size;
    if (newblock_size > MINFRAGMENT)
    {
        // there will be a free fragment after the allocated block
        segment_t base_segment     = pointerToSegment(base);
        segment_t newblock_segment = base_segment + (size / PARAGRAPH_SIZE);

        memblock_t __far* newblock = segmentToPointer(newblock_segment);
        newblock->size = newblock_size;
        newblock->tag  = 0;
        newblock->user = NULL; // NULL indicates free block.
        newblock->next = base->next;
        newblock->prev = base_segment;
#if defined ZONEIDCHECK
        newblock->id   = ZONEID;
#endif

        segmentToPointer(base->next)->prev = newblock_segment;
        base->size = size;
        base->next = newblock_segment;
    }

    base->tag  = tag;
    if (user)
        base->user = user;
    else
        base->user = (void __far*__far*) D_MK_FP(0,2); // unowned
#if defined ZONEIDCHECK
    base->id  = ZONEID;
#endif

    // next allocation will start looking here
    mainzone_rover_segment = base->next;

#if defined INSTRUMENTED
    running_count += base->size;
    printf("Alloc: %ld (%ld)\r\n", base->size, running_count);
#endif

#if defined _M_I86
    memblock_t __far* block = (memblock_t __far*)(((uint32_t)base) + 0x00010000);
#else
    memblock_t __far* block = (memblock_t __far*)(((uint32_t)base) + 0x00010);
#endif

    return block;
}


static void __far* Z_Malloc(uint16_t size, int8_t tag, void __far*__far* user) {
	void __far* ptr = Z_TryMalloc(size, tag, user);
	if (!ptr)
		I_Error ("Z_Malloc: failed to allocate %u B, max free block %li B, total free %li", size, Z_GetLargestFreeBlockSize(), Z_GetTotalFreeMemory());
	return ptr;
}


void __far* Z_TryMallocStatic(uint16_t size)
{
	return Z_TryMalloc(size, PU_STATIC, NULL);
}


void __far* Z_MallocStatic(uint16_t size)
{
	return Z_Malloc(size, PU_STATIC, NULL);
}


void __far* Z_MallocStaticWithUser(uint16_t size, void __far*__far* user)
{
	return Z_Malloc(size, PU_STATIC, user);
}


void __far* Z_MallocLevel(uint16_t size, void __far*__far* user)
{
	return Z_Malloc(size, PU_LEVEL, user);
}


void __far* Z_CallocLevel(uint16_t size)
{
    void __far* ptr = Z_Malloc(size, PU_LEVEL, NULL);
    _fmemset(ptr, 0, size);
    return ptr;
}


void __far* Z_CallocLevSpec(uint16_t size)
{
	void __far* ptr = Z_Malloc(size, PU_LEVSPEC, NULL);
	_fmemset(ptr, 0, size);
	return ptr;
}


boolean Z_IsEnoughFreeMemory(uint16_t size)
{
	const uint8_t __far* ptr = Z_TryMallocStatic(size);
	if (ptr)
	{
		Z_Free(ptr);
		return true;
	} else
		return false;
}


//
// Z_FreeTags
//
void Z_FreeTags(void)
{
    memblock_t __far* next;

    segment_t mainzone_sentinal_segment = pointerToSegment(mainzone_sentinal);

    for (memblock_t __far* block = segmentToPointer(mainzone_sentinal->next); pointerToSegment(block) != mainzone_sentinal_segment; block = next)
    {
        // get link before freeing
        next = segmentToPointer(block->next);

        // already a free block?
        if (!block->user)
            continue;

        if (PU_LEVEL <= block->tag && block->tag <= (PU_PURGELEVEL - 1))
            Z_FreeBlock(block);
    }
}

//
// Z_CheckHeap
//
void Z_CheckHeap (void)
{
    segment_t mainzone_sentinal_segment = pointerToSegment(mainzone_sentinal);

    for (memblock_t __far* block = segmentToPointer(mainzone_sentinal->next); ; block = segmentToPointer(block->next))
    {
        if (block->next == mainzone_sentinal_segment)
        {
            // all blocks have been hit
            break;
        }

#if defined ZONEIDCHECK
        if (block->id != ZONEID)
            I_Error("Z_CheckHeap: block has id %x instead of ZONEID", block->id);
#endif

        if (pointerToSegment(block) + (block->size / PARAGRAPH_SIZE) != block->next)
            I_Error ("Z_CheckHeap: block size does not touch the next block");

        if (segmentToPointer(block->next)->prev != pointerToSegment(block))
            I_Error ("Z_CheckHeap: next block doesn't have proper back link");

        if (!block->user && !segmentToPointer(block->next)->user)
            I_Error ("Z_CheckHeap: two consecutive free blocks");
    }
}
