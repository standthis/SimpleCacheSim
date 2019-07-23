/* cachesetup.c
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

#include "cachesetup.h"
#include "stringutils.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>

struct CacheSetup {
    CachesizeT totalblocks;
    BlocksizeT blocksize;
    LatencyT hittime,
             lookupoverhead;
    CacheAssociativityT associativity;
    bool split;
}; // typedef CacheSetupT

// add a new set of cache parameters to the list: because of realloc, should
// assign the result back to the original data structure; inserts the given
// pointer and does not copy so it should not be freed outside of managing this
// data structure
CacheSetupT **addCacheParameters (CacheSetupT* allparemeters[], CacheSetupT *newone) {
    int Nparameters = parameterlen (allparemeters);
    allparemeters = realloc (allparemeters, sizeof(CacheSetupT*)*(Nparameters+2));
    allparemeters[Nparameters] = newone;
    allparemeters[Nparameters+1] = NULL;
    return allparemeters;
}

CacheSetupT *makeCacheParameters (CachesizeT totalblocks,
    BlocksizeT blocksize,
    LatencyT hittime, LatencyT lookupoverhead,
    CacheAssociativityT associativity, bool split) {
    
    CacheSetupT *newparameters = malloc (sizeof(CacheSetupT));
    newparameters->totalblocks = totalblocks;
    newparameters->blocksize = blocksize;
    newparameters->hittime = hittime;
    newparameters->lookupoverhead = lookupoverhead;
    newparameters->associativity = associativity;
    newparameters->split = split;
    return newparameters;

}

// display paraemters in neat format for reporting
void reportParameters (CacheSetupT* allparemeters[]) {
    int Nparameters = parameterlen (allparemeters);
    int level = 1;
    if (!Nparameters) {
        fprintf(stderr, "No parameters\n");
        return;
    }
    printf ("\tblks\tblksize\thitT\tlookupT\tassoc\tsplit?\tTotal Bytes\n");
    bool splitL1 = allparemeters[0]->split;
    for (int i = 0; i < Nparameters-1; i++) {
        printf ("L%d%s:\t%u\t%u\t%lu\t%lu\t%u\t%d\t%u\n", level,
                splitL1?(i==0?"I":(i==1?"D":"")):"",
                allparemeters[i]->totalblocks,
                allparemeters[i]->blocksize,
                allparemeters[i]->hittime,
                allparemeters[i]->lookupoverhead,
                allparemeters[i]->associativity,
                (allparemeters[i]->split?1:0),
                allparemeters[i]->totalblocks*
                allparemeters[i]->blocksize
               );
        if (i > 0)
            level++;
        else if (!splitL1)
            level++;
    }
    printf ("DRAM:\t\t\t%lu\n", allparemeters[Nparameters-1]->hittime);
}

// a line in the form of a null-terminated string containing:
// cache size in bytes, block size in bytes, hit time, miss overhead, associativity
// and whether spit I+D (1 split, 0 not: should only be the case in L1)
CacheSetupT *makeCacheParametersStr (char *line) {
    if (isnumbers (line)) {
        CachesizeT totalsize;
        BlocksizeT blocksize;
        LatencyT hittime,
        lookupoverhead;
        CacheAssociativityT associativity;
        int splitasint;
        bool split;
        if (sscanf(line, "%u %u %lu %lu %u %d", &totalsize, &blocksize, &hittime,
            &lookupoverhead, &associativity, &splitasint) == 6) {
            CachesizeT nBlocks = blocksize?totalsize/blocksize:totalsize;
            if (blocksize && totalsize % blocksize)
               error (configError, false, "Total size not a multiple of block size",
                      __LINE__, __FILE__);
            return makeCacheParameters (nBlocks, blocksize, hittime,
                                        lookupoverhead, associativity, splitasint!=0);
        }
       
    }
    // only get here if an error in parameters
    error (configError, false, line, __LINE__, __FILE__);
    return NULL; // keep compilers that check for return value happy, error exits
}

// Maintaining inclusion gets complicated if block sizes vary
// if higher-evel bigger, OK; if higher level smaller, every eviction
// from a lower level with bigger blocks must evict any at higher level
// that overlap it
void checkparameters (CacheSetupT * caches []) {
    for (int i = 1; caches[i]; i++) {
        CacheSetupT * thiscache = caches[i], *lastcache = caches[i-1];
        // none of these can be zero except in DRAM layer
        if (!thiscache->totalblocks || !thiscache->blocksize ||
           !thiscache->associativity) {
              if (caches[i+1]) // not in DRAM layer if there is another layer below this
                 error (configError, false,
                        "Only DRAM layer may have zeros in block attributes",
                        __LINE__, __FILE__);
        }
#ifdef DEBUG
        fprintf (stderr, "Checking %d vs. %d: %u == %u\n", i, i-1,
                 thiscache->blocksize, lastcache->blocksize);
#endif
    }
}

CacheSetupT** getconfig (char **configlines) {
    CacheSetupT** setup = NULL;
    for (int i = 0; configlines[i]; i++)
         setup = addCacheParameters (setup, makeCacheParametersStr(configlines[i]));
    return setup;
}

void deconstruct_setup (CacheSetupT * caches []) {
    for (int i = 0; caches[i]; i++) // stop on NULL
        free (caches[i]);
    free (caches);
}


int parameterlen (CacheSetupT* allparemeters[]) {
    int Nparameters = 0;
    if (allparemeters)
        for (int i = 0; allparemeters[i]; i++) {
            Nparameters++;
        }
    return Nparameters;
}

//////////////////////////////////// CacheSetup acccess functions ///////////////////////////////////
CachesizeT getSetupTotalblocks (CacheSetupT *setup) {
   return setup->totalblocks;
}

BlocksizeT getSetupBlocksize (CacheSetupT *setup) {
   return setup->blocksize;
}

LatencyT getSetupHittime (CacheSetupT *setup) {
   return setup->hittime;
}

LatencyT getSetupLookupoverhead (CacheSetupT *setup) {
   return setup->lookupoverhead;
}

CacheAssociativityT getSetupAssociativity (CacheSetupT *setup) {
   return setup->associativity;
}

bool getSetupSplit (CacheSetupT *setup) {
   return setup->split;
}
