/* multilevelAssoc.h
 * 
 * Types and code to simulate a multilevel associative cache: we need not store the
 * data in blocks to record number and timing of accesses.
 *
 * Philip Machanick
 * June 2018
 * 
 */

#ifndef multilevelAssoc_h
#define multilevelAssoc_h

#include <stdbool.h>

#include "stats.h"
#include "rawcache.h"

typedef struct Cache CacheT;       // can be multilevel, multiway

#include "readtrace.h"
#include "generaltypes.h"

//define details of types in C file for abstraction

// collection of stats data structures
typedef struct AllStats AllStatsT;


typedef struct Cacheblock CacheblockT;

#include "cachesetup.h"

int countlevels (CacheT *cache[]);

bool is_split (CacheT *cache);

// for multilevel associative cache: true if found in L1
bool cacheHit (CacheT* thecache[], AddressT where, ReftypeT reftype);

// for multilevel associative cache: report level where found (1 more than max if not)
int findInCache (CacheT* thecache[], AddressT where, ReftypeT reftype);

void handleReference (CacheT* thecache[], AddressT where, ReftypeT reftype);

// check for hits, find victim and find an empty slot taking into account
// associativity in a given level of cache
CacheAssociativityT assocFindVictim (CacheT* cache);
CacheAssociativityT assocFindEmpty (CacheT* cache, AddressT address);

// set up mutliple cache levels; top level can be split
// and bottom level always finds its referenece 1 level down (RAM)
// each cache represented in an array with item 0 for L1, LLC last
// item -- terminated by NULL pointer
CacheT** initmultilevelcache (CacheSetupT * caches []);

CacheAssociativityT assocCacheHit (CacheT* thecache, AddressT where);

// create a new cache including memory allocation; all blocks initially invalid
CacheT* initcache (CachesizeT blocks, BlocksizeT blocksize);

void deconstruct_multilevelcache (CacheT** caches);

void reportstats (CacheT *cache[]);

#endif // multilevelAssoc_h
