/* cachesetup.h
 *
 * Create and use data structure containing cache paramenters
 * requires a configuration file to have been read into a
 * data structure that represents each line as a string pointed
 * to by an array of char*
 *
 * Philip Machanick
 * June 2018
 *
 */

#ifndef cachesetup_h
#define cachesetup_h

#include "generaltypes.h"
#include "rawcache.h"

typedef struct CacheSetup CacheSetupT;

// add a new set of cache parameters to the list: because of realloc, should
// assign the result back to the original data structure; inserts the given
// pointer and does not copy so it should not be freed outside of managing this
// data structure
CacheSetupT **addCacheParameters (CacheSetupT* allparemeters[], CacheSetupT *newone);

CacheSetupT *makeCacheParameters (CachesizeT totalblocks,
    BlocksizeT blocksize,
    LatencyT hittime, LatencyT missoverhead,
    CacheAssociativityT associativity, bool split);

CacheSetupT *makeCacheParametersStr (char *line);

void checkparameters (CacheSetupT * caches []);

CacheSetupT** getconfig (char **configlines);

// deallocate an array of cache parameters including the outer level
void deconstruct_setup (CacheSetupT * caches []);

// display paraemters in neat format for reporting
// labelled with level (and I or D if L1 split) with
// block count as well as total bytes
void reportParameters (CacheSetupT* allparemeters[]);


int parameterlen (CacheSetupT* allparemeters[]);

CachesizeT getSetupTotalblocks (CacheSetupT *setup);

BlocksizeT getSetupBlocksize (CacheSetupT *setup);

LatencyT getSetupHittime (CacheSetupT *setup);

LatencyT getSetupLookupoverhead (CacheSetupT *setup);

CacheAssociativityT getSetupAssociativity (CacheSetupT *setup);

bool getSetupSplit (CacheSetupT *setup);


#endif // cachesetup_h
