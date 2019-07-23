/*
 * rawcache.c
 * 
 * Types and operations to simulate a DM cache: we need not store the
 * data in a block to track accesses. Build on this to implement a
 * timed cache with attributes like associativity > 1 or multiple levels.
 *
 * Philip Machanick
 * June 2018
 * 
 */

#include "rawcache.h"
#include "error.h"

#include <stdlib.h> // malloc
#include <stdio.h>  // for testing

/////////////////////////////////////// LOCAL TYPES //////////////////////////////////////
//////////////////////////////// DETAIL HIDDEN FROM HEADER ///////////////////////////////

typedef unsigned Bitshift;

struct Cacheblock {
  TagT tags;
  AddressT addressbits;
  // for simulation we can ignore the data
}; // typedef CacheblockT

typedef struct RawCache {
  CachesizeT Nblocks;
  BlocksizeT blocksize;
  CacheblockT *blocks;
  AddressT addressmask,
           indexmask;
  Bitshift offsetbits,
           indexbits;
} RawCacheT;


//////////////////////////////////// STATIC PROTOTYPES ///////////////////////////////////

static CachesizeT storedaddress (RawCacheT* thecache, AddressT where);

static void rawinvalidate (RawCacheT* thecache, CachesizeT whichblock);

static void invalidblock (CacheblockT * block);

// turn on given tags leaving any others unchanged
static void settags (CacheblockT * block, TagT tags);

// only turn on given tags, ensure the rest are 0
static void settagsexclusive (CacheblockT * block, TagT tags);

static AddressT getAddressMask (BlocksizeT blocksize);

// given an address of a cache block, a mask that removes the high
// bits over and above those needed for indexing the cache 
static AddressT getIndexMask (CachesizeT Nblocks);

static Bitshift calculateIndexBits (CachesizeT Nblocks);

static Bitshift calculateOffsetBits (BlocksizeT blocksize);


//////////////////////////////////// GLOBAL FUNCTIONS ////////////////////////////////////
//////////////////////////////////// VISIBLE IN HEADER ///////////////////////////////////

// initialize raw cache
RawCacheT* initrawcache (CachesizeT blocks, BlocksizeT blocksize) {
    if (!checkPowerof2 (blocks))
        error (badblockcount, true, "Block count must be a power of two", __LINE__, __FILE__);
    if (!checkPowerof2 (blocksize))
        error (badblockcount, 0, "Block size must be a power of two", __LINE__, __FILE__);

    RawCacheT *newcache = malloc (sizeof (RawCacheT));
    newcache->Nblocks = blocks;
    newcache->blocksize = blocksize;
    // allocate blocks memory
    newcache->blocks = malloc (sizeof (CacheblockT)*blocks);
    // calculate offsetbits and addressmask
    newcache->offsetbits = calculateOffsetBits (blocksize);
    newcache->indexbits = calculateIndexBits (blocks);
    newcache->addressmask = getAddressMask (blocksize);
    newcache->indexmask = getIndexMask (blocks);
    // invalidate all blocks for correct initial state 
    for (int i = 0; i < blocks; i++) {
        rawinvalidate (newcache, i); // all bits off
    }
    return newcache;
}

BlocksizeT getblocksize (RawCacheT *cache) {
   return cache->blocksize;
}

void deconstruct_cache (RawCacheT **cache) {
   free ((*cache)->blocks);
   free (*cache); // pointer now invalid so set it NULL
   *cache = NULL;
}

void setbits (RawCacheT* thecache, CachesizeT whichblock, TagT tags) {
    if (whichblock < thecache->Nblocks)
        settags (&(thecache->blocks[whichblock]), tags);
    else
        error (badblockindex, true, "Block index out of range", __LINE__, __FILE__);
}

void invalidate (RawCacheT* thecache, AddressT where) {
   CachesizeT whichblock = blockaddress (thecache, where);
   rawinvalidate (thecache, whichblock);
}

// to determine a hit we need to find the right block
// in a Assoc cache that just means stipping off the offset bits and using enough
// of the remaining bits to form an index; in N-way set associative, we need
// to treat this as if ew have N caches each 1/N of the size, look up in each
// and find whether there is a hit in each
bool rawCacheHit (RawCacheT* thecache, AddressT where) {
   CachesizeT whichblock = blockaddress (thecache, where);
   if (thecache->blocks[whichblock].tags&VALID) {
       // Valid so check if bits in tag match high bits of address
       AddressT cacheaddrtag = storedaddress (thecache, where);
#ifdef DEBUG
       fprintf(stderr,"new tag 0x%x, stored 0x%x\n", cacheaddrtag, thecache>blocks[whichblock].addressbits);
#endif
       return (cacheaddrtag == thecache->blocks[whichblock].addressbits);
   } else
       return false;
}

CachesizeT getNblocks (RawCacheT* cache) {
   return cache->Nblocks;
}

bool rawcachecheck (RawCacheT *cache) {
    for (int block = 0; block < getNblocks (cache); block++)
         if ((cache->blocks[block].tags & VALID) && 
            (!(cache->blocks[block].addressbits)))
              return false;
    return true;
}

// assume we can use this block  -- call after checking and if necessary
// invalidating or writing back
void insert (RawCacheT* thecache, AddressT where) {
     CachesizeT whichblock = blockaddress (thecache, where);
     settagsexclusive (&thecache->blocks[whichblock], VALID); // only VALID bit on
     thecache->blocks[whichblock].addressbits =
         storedaddress (thecache, where);
if (!thecache->blocks[whichblock].addressbits) printf("0 addr tag, address is 0x%x\n", where);
#ifdef DEBUG
     fprintf(stderr, "offset bits %u, index mask 0x%x; storing addr 0x%x at block 0x%x with address bits 0x%x\n",
             thecache->offsetbits, thecache->indexmask,
             where, whichblock, thecache->blocks[whichblock].addressbits);
#endif
}

// if the cache block is modified, signal a writeback but otherwise do nothing
bool mustWriteback (RawCacheT* thecache, AddressT where) {
    TagT statusbits = status (thecache, where);
    if (! (statusbits & VALID))
       return false;
    if (! (statusbits & MODIFIED))
       return false;
    return true;
}

// return status of a cache block
TagT status (RawCacheT* thecache, AddressT where) {
   CachesizeT whichblock = blockaddress (thecache, where);
   return thecache->blocks[whichblock].tags;
}

// which block does this address fall in?
CachesizeT blockaddress (RawCacheT* thecache, AddressT where) {
    return ((thecache->addressmask & where) >> thecache->offsetbits) & thecache->indexmask;
}

// returns true only if passed in val is a power of 2
// a power of 2 will only have 1 bit set so if we left shift until the value
// is 0 and add the leftmost bit to a count, only a power of 2 will result
// in count exactly 1 (0 isn't a power of anything and not interesting here)
bool checkPowerof2(unsigned val) {
    unsigned count = 0;
    while (val) {
        count += val & 1;
        val >>= 1;
    }
    return (count == 1);
}

// create a mask that for a power of 2, 2^N, is N low bits set to 1
TagT getMask (TagT value) {
  TagT mask = 0;
  while (value >>= 1) {
    mask <<= 1;
    mask |= 1;
  }
  return mask;
}

AddressT getTagAddressBits (RawCacheT* thecache, unsigned index) {
   return thecache->blocks[index].addressbits;
}

// this will not give the start address of the block
AddressT tagToAddress (RawCacheT* thecache, unsigned index) {
    AddressT addressbits = thecache->blocks[index].addressbits;
    return (((addressbits << thecache->indexbits)  | index ) << 
              thecache->offsetbits);
}


//////////////////////////////////// STATIC FUNCTIONS ////////////////////////////////////

static void rawinvalidate (RawCacheT* thecache, CachesizeT whichblock) {
    if (whichblock <= thecache->Nblocks)
        invalidblock (&(thecache->blocks[whichblock]));
    else 
        error (badblockindex, false, "Block index out of range", __LINE__, __FILE__);
}

static void invalidblock (CacheblockT * block) {
    block->tags = INVALID;  // turn off all bits including VALID
    block->addressbits = 0; // unnecessary but safer
}

static CachesizeT storedaddress (RawCacheT* thecache, AddressT where) {
    return (where >> thecache->offsetbits) >> thecache->indexbits;
}

// turn on given tags leaving any others unchanged
static void settags (CacheblockT * block, TagT tags) {
    block->tags |= tags; // turn on given tags, leave rest uncahnged
}

// only turn on given tags, ensure the rest are 0
static void settagsexclusive (CacheblockT * block, TagT tags) {
    block->tags = tags; // turn on only given tags, turn off rest
}

static AddressT getAddressMask (BlocksizeT blocksize) {
    AddressT addressmask = 0;
    // find all the positions that we want to remove first -- if block size is 2^N
    // we don't want the low bits that would be zero in 2^N
    // Then: invert the bits to create a mask that will remove the low N bits
    // add an extra 1 into the mask each time we can shift blocksize left
    blocksize >>= 1; // first eliminate 1 bit
    while (blocksize) {
       addressmask <<= 1;
       addressmask |= 1;
       blocksize >>= 1;
    }
    return ~addressmask;
}

// given an address of a cache block, a mask that removes the high
// bits over and above those needed for indexing the cache 
static AddressT getIndexMask (CachesizeT Nblocks) {
    AddressT bits = 0;
    Nblocks >>= 1;
    while (Nblocks) {
        bits <<= 1;
        bits |= 1;
        Nblocks >>= 1;
    }
    return bits;
}

static Bitshift calculateIndexBits (CachesizeT Nblocks) {
    Bitshift bits = 0;
    while (true) {
        Nblocks >>= 1;
        if (!Nblocks) break;
        bits++;
    }
    return bits;
}

static Bitshift calculateOffsetBits (BlocksizeT blocksize) {
    Bitshift bits = 0;
    while (true) {
        blocksize >>= 1;
        if (!blocksize) break;
        bits++;
    }
    return bits;
}

//////////////////////////////////// UNIT TEST DRIVER ////////////////////////////////////

#ifdef UNITTESTRAWCACHETYPES

// FIXME needs rework since splitting off raw cache

// only compiled if compiler line has -DUNITTESTRACACHETYPES -- needs to link with
// error.o and stringutils.o


const Trace tracerecord[] = {
    {READ, 1},
    {READ, 31},
    {READ, 524288},  // 16384*32 to wrap around and get the same place direct-mapped
    {READ, 1048576}, // wrap aroun again, 2-way must miss
    {WRITE, 128},
    {READ, 128},
    {FETCH, 256}
};


int main () {

    
}

#endif // UNITTESTRAWCACHETYPES
