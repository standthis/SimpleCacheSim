/*
 * rawcache.h
 * 
 * Types and operations to simulate a DM cache: we need not store the
 * data in a block to track accesses. Build on this to implement a
 * timed cache with attributes like associativity > 1 or multiple levels.
 *
 * Philip Machanick
 * June 2018
 * 
 */

#ifndef rawcache_h
#define rawcache_h

#include <stdbool.h>

typedef struct RawCache RawCacheT; // single way

typedef unsigned CachesizeT;
typedef unsigned BlocksizeT;

typedef unsigned TagT;
typedef unsigned AddressT;

// tag bits: not totally portable as C compilers differ on type of enum values
// but should be correct for all the bits we need
enum tagvalues {
    INVALID   = 0, // assign this to initialize
    VALID     = 1,
    MODIFIED  = 2,
    SHARED    = 4,
    EXCLUSIVE = 8
};


//define details of types in C file for abstraction
typedef struct Cacheblock CacheblockT;

// true if found in a DM cache
bool rawCacheHit (RawCacheT* thecache, AddressT where);

CachesizeT getNblocks (RawCacheT* cache);

bool rawcachecheck (RawCacheT *cache);

// create a basic DM cache, ususally embedded in something else
RawCacheT* initrawcache (CachesizeT blocks, BlocksizeT blocksize);

BlocksizeT getblocksize (RawCacheT *cache);

// deallocate memory used for the cache -- pass in pointer so we can change
// the value of the actual cache variable
void deconstruct_cache (RawCacheT** cache);

// change state of a block to invalid
void invalidate (RawCacheT* thecache, CachesizeT whichblock);

// set any combination of tag bits leaving rest unchanged
void setbits (RawCacheT* thecache, CachesizeT whichblock, TagT tags);

// find out which block an address maps to
CachesizeT blockaddress (RawCacheT* thecache, AddressT where);

// insert in cache: assumes the block to be inserted has already
// been checked e.g. for invalidation or writing back
void insert (RawCacheT* thecache, AddressT where);

// if the cache block is modified, signal a writeback but otherwise do nothing
bool mustWriteback (RawCacheT* thecache, AddressT where);

// return status of a cache block
TagT status (RawCacheT* thecache, AddressT where);

// returns true only if passed in val is a power of 2
// a power of 2 will only have 1 bit set so if we left shift until the value
// is 0 and add the leftmost bit to a count, only a power of 2 will result
// in count exactly 1 (0 isn't a power of anything and not interesting here)
bool checkPowerof2(unsigned val);

// create a mask that for a power of 2, 2^N, is N low bits set to 1
TagT getMask (TagT value);

// get lowest address this tag index could fall in (useful for finding
// range of blocks in a level with smaller blocks)
AddressT tagToAddress (RawCacheT* thecache, unsigned index);

// debug: not usually needed
AddressT getTagAddressBits (RawCacheT* thecache, unsigned index);

#endif // rawcache_h
