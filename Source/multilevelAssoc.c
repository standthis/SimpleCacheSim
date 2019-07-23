/*
 * multilevelAssoc.c
 * 
 * Types and code to simulate a multilevel associative cache: we need not store the
 * data in blocks to record number and timing of accesses.
 *
 * Philip Machanick
 * June 2018
 * 
 */

#include "multilevelAssoc.h"
#include "error.h"
#include "stringutils.h"

#include <stdlib.h> // malloc
#include <stdio.h>  // for testing

/////////////////////////////////////// LOCAL TYPES //////////////////////////////////////
//////////////////////////////// DETAIL HIDDEN FROM HEADER ///////////////////////////////

typedef unsigned Bitshift;

// one raw cache for each way; for a split cache add another
// cache; from top down, cache[0] is L1I cache, cache[1] is L2D
// if split; from there down, cache[i+1] is the next level down
// from cache[i]
struct Cache {
    RawCacheT **cachedata;
    LatencyT hittime,
             lookupoverhead;
    CacheAssociativityT associativity;
    bool split;
    TagT assocmask;
    AllStatsT *stats;
};  //typedef CacheT

struct AllStats {
   StatsT *hitcount,
          *misscount,
          *replacecount,
          *inclusioncount,
          *hitcost,
          *misscost;
};


//////////////////////////////////// STATIC PROTOTYPES ///////////////////////////////////

// deal with a cache miss: place at all levels to maintain inclusion
static void handleMiss (CacheT* thecache[], AddressT where, ReftypeT reftype, int foundat);

static AddressT getAddressMask (BlocksizeT blocksize);

// given an address of a cache block, a mask that removes the high
// bits over and above those needed for indexing the cache 
static AddressT getIndexMask (CachesizeT Nblocks);

static Bitshift calculateIndexBits (CachesizeT Nblocks);

static Bitshift calculateOffsetBits (BlocksizeT blocksize);

static AllStatsT* init_all_stats ();
static void deconstruct_all_stats (AllStatsT* stats);

static void dowrite (CacheT *level, AddressT where);
static void maintaininclusion (CacheT* multilevelcache [], int misslevel, AddressT where,
                               ReftypeT reftype);


//////////////////////////////////// GLOBAL FUNCTIONS ////////////////////////////////////
//////////////////////////////////// VISIBLE IN HEADER ///////////////////////////////////

int countlevels (CacheT *cache[]) {
    int Nlevels = 0;
    for (int i = 0; cache[i]; i++) {
        if (!cache[i]->associativity)
            break; // don't count main memory as a level: signal ass assoc==0
        Nlevels++;
    }
    if (cache[0]->split)
        Nlevels--;
    return Nlevels;
}

bool is_split (CacheT *cache) {
    if (cache) {
        return cache->split;
    }
    else
        return false;
}

static CacheT * initAssocCache (CacheSetupT *cacheinfo) {
    CacheAssociativityT associativity = getSetupAssociativity (cacheinfo);
    if (associativity && !checkPowerof2(associativity))
        error (badAssociativity, true, "Cache associativity should be a power of 2",
               __LINE__, __FILE__);
    CachesizeT totalblocks = getSetupTotalblocks (cacheinfo);
    BlocksizeT blocksize = getSetupBlocksize (cacheinfo);
    LatencyT hittime = getSetupHittime (cacheinfo);
    LatencyT lookupoverhead = getSetupLookupoverhead (cacheinfo);
    bool split = getSetupSplit (cacheinfo);
    if (associativity && totalblocks % associativity)
        error (badAssociativity, false, "Associativity cache blocks don't divide evenly between ways",
               __LINE__, __FILE__);

    CacheT * assoccache = malloc(sizeof(CacheT));
    if (associativity) {
        assoccache->cachedata = malloc(sizeof(RawCacheT*)*associativity);
        for (int i = 0; i < associativity; i++) {
            assoccache->cachedata[i] = 
            initrawcache (totalblocks/associativity,
                          blocksize);
         }
         assoccache->assocmask = getMask (associativity);
    } else {
        assoccache->cachedata = NULL; // should only happen with main memory
    }
    assoccache->hittime = hittime;
    assoccache->lookupoverhead = lookupoverhead;
    assoccache->associativity = associativity;
    assoccache->split = split;
    assoccache->stats = init_all_stats ();
    return assoccache;
}

// set up mutliple cache levels; top level can be split
// and bottom level always finds its referenece 1 level down (infinite RAM)
// each cache represented in an array with item 0 for L1 with LLC last
// item -- terminated by NULL pointer
// For split L1, first 2 parameter sets should be for L1I and L1D in that order
// for split L1, newcaches[0] and newcaches[1] constitute L1;
// for unified L1, L2 starts at newcaches[1]
    
CacheT** initmultilevelcache (CacheSetupT * caches []) {
    // int startL2 = 1; // where to start L2
    int Ncaches = 0;
    checkparameters (caches);
    // count how many cache data structures needed: end on NULL entry
    for (Ncaches = 0; caches[Ncaches]; Ncaches++) ;
    CacheT** newcaches = malloc (sizeof(CacheT*)*(Ncaches+1));
    // for split L1, newcaches[0] and newcaches[1] constitute L1;
    // for unified L1, L2 starts at newcaches[1]
    for (int i = 0; caches[i]; i++) {
        newcaches[i] = initAssocCache (caches[i]);
    }
    newcaches[Ncaches] = NULL; // mark the end
    return newcaches;
}

void deconstruct_multilevelcache (CacheT** caches) {
    for (int Ncaches = 0; caches[Ncaches]; Ncaches++) {
        deconstruct_all_stats (caches[Ncaches]->stats);
        free(caches[Ncaches]);
    }
    free (caches);
}

CacheAssociativityT assocCacheHit (CacheT* thecache, AddressT where) {
    for (int i = 0; i < thecache->associativity; i++) {
        if (rawCacheHit (thecache->cachedata[i], where))
            return i;
    }
    return thecache->associativity; // miss
}

// pass in a pointer to the cache array at the level of interest
// if L1, point to element 0 and if split, use element 1 instead
// for any other level, split is expected to be 0
bool cacheHit (CacheT* thecache[], AddressT where, ReftypeT reftype) {
    int whichsplit = 0;
    // reference type is FETCH: cache index0 otherwise 1
    if (thecache[0]->split)
        whichsplit = (reftype == FETCH)?0:1;
    if (assocCacheHit (thecache[whichsplit], where) <
             thecache[whichsplit]->associativity)
        return true;

    return false;
}

static int maxfoundat = 0;
static int maxI = 0;
#ifdef DEBUG
static LatencyT maxhitcost = 0;
#endif


// find highest cache level where an address is represented; if in L1, no cost since that
// is accounted for in a hit; if lower, take max cost of lookup since the hardware can do
// lookups in parallel;
// if not in any level, no cost to check if in main memory since we are not modelling VM so
// everything is assumed to be in DRAM; only DRAM cost is copying to LLC
int findInCache (CacheT* thecache[], AddressT where, ReftypeT reftype) {
    int startL2 = 1;
    int offEdge = countlevels (thecache); // 1 more than highest index in cache array
    int foundat = offEdge;
    int L1Dindex = 0, L1Iindex = 0;
    LatencyT lookupcost = thecache[L1Iindex]->lookupoverhead; // correct for split if L1D

    if (thecache[0]->split) {
        startL2++;
        offEdge++;
        foundat++;
        L1Dindex = 1;
    }
    // first check if in L1; no cost for this here, accounted for in handleReference
    if (reftype == FETCH) {
        if (assocCacheHit (thecache[L1Iindex], where) < thecache[L1Iindex]->associativity)
           return L1Iindex;
    } else {  // if not a fetch, need to correct lookupcost
        if (assocCacheHit (thecache[L1Dindex], where) < thecache[L1Dindex]->associativity)
           return L1Dindex;
        lookupcost = thecache[L1Dindex]->lookupoverhead; // need this now
    }
#ifdef DEBUG
    fprintf(stderr, "Not found in L1: 0x%x [%c]", where, reftype);
#endif
    // find where the item actually is and incur the cost of looking it up; since
    // we assume infinite main memory, there is no need to look up in DRAM
    for (int i = startL2; i < offEdge; i++) {
        // inclur the lookup overhead at each level: in a real cache done in parallel
        // so only score the max value
        if (i > maxI) maxI = i;
        if (thecache[i]->lookupoverhead > lookupcost)
            lookupcost = thecache[i]->lookupoverhead;
        if (assocCacheHit (thecache[i], where) < thecache[i]->associativity) {
            AllStatsT * stats = thecache[i]->stats;
            LatencyT hitcost = thecache[i]->hittime;
#ifdef DEBUG
            if (hitcost > maxhitcost) maxhitcost = hitcost;
#endif
           foundat = i;
           break;
        }
    }
    LatencyT hitcost = thecache[foundat]->hittime;
    if (reftype == FETCH) {
        incrIcost (thecache[L1Dindex]->stats->misscost, hitcost);
        if (foundat < offEdge) {
          // incrIcost (thecache[foundat]->stats->hitcost, hitcost);
           incrIcount (thecache[foundat]->stats->hitcount);
        }
    } else if (reftype == READ) {
        incrDRcost (thecache[L1Dindex]->stats->misscost, hitcost);
        if (foundat < offEdge) {
          // incrDRcost (thecache[foundat]->stats->hitcost, hitcost);
           incrDRcount (thecache[foundat]->stats->hitcount);
        }
    } else {
        incrDWcost (thecache[L1Dindex]->stats->misscost, hitcost);
        if (foundat < offEdge) {
           //incrDWcost (thecache[foundat]->stats->hitcost, hitcost);
           incrDWcount (thecache[foundat]->stats->hitcount);
        }
    }

#ifdef DEBUG
    fprintf(stderr, "Found at %d: 0x%x [%c]", foundat, where, reftype);
    // not found if foundat still == offEdge
    if (foundat > maxfoundat) { maxfoundat = foundat;
    fprintf (stderr, "#######Foundat max = %d, offEdge = %d, max i = %d\n", foundat, offEdge, maxI);
    }
#endif
    return foundat;
}

#define L1INDEX(cache,reftype) (cache[0]->split?(reftype==FETCH?0:1):0)

static ELAPSED refcount = 0;

// check the cacche has no 0 address tag with VALID set on
static bool cachecheck (CacheT* thecache[]) {
   for (int i = 0; thecache[i]; i++) {
      CacheT * cache = thecache [i];
      for (int j = 0; j < cache->associativity; j++) {
         if (!rawcachecheck (cache->cachedata[j]))
           return false;
      }
   }
   return true;
}

void handleReference (CacheT* thecache[], AddressT where, ReftypeT reftype) {
    int startL2 = thecache[0]->split?2:1; // use to differentiat split and unified L1
    // if L1 split, L1I at cache[0] for fetch and L1D at cache[1], unified L1 at cache[0]
    int foundat = findInCache (thecache, where, reftype), // 0 or 1 if no miss
        indexL1 = L1INDEX(thecache,reftype);
    // add L1 costs here: elsewhere add costs there
    // in L1, no miss costs to account for
    ELAPSED lookupcost = thecache[indexL1]->lookupoverhead,
            hittime = thecache[indexL1]->hittime;
refcount++;
    if (foundat == indexL1) {
#ifdef DEBUG
        fprintf(stderr,"hit 0x%x, hitcost = %lu\n", where, hittime);
#endif
        if (reftype == FETCH) {
            incrIcount (thecache[indexL1]->stats->hitcount);
            incrIcost (thecache[indexL1]->stats->hitcost, hittime);
       } else if (reftype == READ) {
            incrDRcount (thecache[indexL1]->stats->hitcount);
            incrDRcost (thecache[indexL1]->stats->hitcost, hittime);
        } else if (reftype == WRITE) {
            incrDWcount (thecache[indexL1]->stats->hitcount);
            incrDWcost (thecache[indexL1]->stats->hitcost, hittime);
        }
    } else {  // a miss: add cost of L1 reference on a miss
         // account for the cost of copying to here from layer where block is found
         LatencyT misscost = thecache[foundat]->hittime;
         if (reftype == FETCH) {
            incrIcost (thecache[indexL1]->stats->misscost, hittime);
         } else if (reftype == READ) {
            incrDRcost (thecache[indexL1]->stats->misscost, hittime);
         } else if (reftype == WRITE) {
            incrDWcost (thecache[indexL1]->stats->misscost, hittime);
         }
         handleMiss (thecache, where, reftype, foundat);
    }
}

static void handleMiss (CacheT* thecache[], AddressT where, ReftypeT reftype, int foundat) {
    bool split = thecache[0]->split;
    int startL2 = split?2:1; // use to differentiat split and unified L1
    int offEdge = countlevels (thecache); // 1 more than highest index in cache array
    int indexL1D = startL2-1;

    if (split) {
        offEdge++;
    }
    // place at each cache level above where it was found to maintain inclusion
    // any replacements also have to be done so as to maintain inclusion; doing
    // this from lowest level up increases the chances that we do not need to
    // replace again as we move up, as lower-level replacements should open up
    // a spot higher up (not always: if the higher level cache is less associative
    // the lower-level eviction may not necessarily map to the same block). If foundat
    // is 1 off the edge, we get a miss from LLC.
    for (int level = foundat-1; level >= indexL1D; level--) {
        int i = level;
        if (split && (reftype == FETCH) && (i == indexL1D))
            i--;  // i == 1 for L1, fetch handled differently for split cache
        CacheAssociativityT associativity = thecache[i]->associativity;
        CacheAssociativityT candidate = assocFindEmpty (thecache[i], where);
//printf("placing 0x%x in $[%d]\n", where, level);
#ifdef DEBUG
        fprintf(stderr,"placing 0x%x in $[%d] ", where, level);
#endif
        if (candidate >= associativity) { // none invalid, choose a victim
            candidate = assocFindVictim (thecache[i]);
            // We need an address that takes us to the block we are evicting:
            // reverse calculation we did to put in the address tag to maintain inclusion
            // since other levels may have a different block size
#ifdef DEBUG
            fprintf(stderr, "replacing in way %d ", candidate);
#endif
            RawCacheT *rawcache = thecache[i]->cachedata[candidate];
            AddressT victimwhere = tagToAddress (rawcache, blockaddress (rawcache, where));
            if (!(status (rawcache, victimwhere) & VALID)) {
                error (associativityError, false, "Associative cache victim should be valid",
                       __LINE__, __FILE__);
            }
            maintaininclusion (thecache, i, victimwhere, reftype);
            // no longer at any upper level, if modified higher up, modified here now
            if (status (rawcache, where) & MODIFIED) {
                dowrite (thecache[i+1], where); // must be at next level down
                
                // incur writeback penalty here; for now
                // assume writebacks fully buffered so no write cost
            }
            invalidate (rawcache, where); // now free to use this block
            if (reftype == WRITE) {
                incrDWcount (thecache[i]->stats->replacecount);
            } else if (reftype == READ) {
                incrDRcount (thecache[i]->stats->replacecount);
            } else {
                incrIcount (thecache[i]->stats->replacecount);
            }
        }
#ifdef DEBUG
        fprintf (stderr, "placing in way %d\n", candidate);
#endif
        // found empty way to put in or doing replacement into cache[candidate]
        // account for cost of finding the place and for reading next level down
        RawCacheT *rawcache = thecache[i]->cachedata[candidate];
        insert (rawcache, where); // make the block valid and set the address bits
        LatencyT lookupcost = thecache[i]->lookupoverhead,
                 misscost = thecache[i+1]->hittime + thecache[i+1]->lookupoverhead;
#ifdef DEBUG
        fprintf(stderr, "Miss at $%d, lookup %ld miss cost %ld\n", i, lookupcost, misscost);
#endif
        if (reftype == WRITE) {
            setbits (rawcache, blockaddress (rawcache, where), MODIFIED);
            incrDWcost (thecache[i]->stats->misscost, lookupcost + misscost);
            incrDWcount (thecache[i]->stats->misscount);
        } else if (reftype == READ) {
            incrDRcost (thecache[i]->stats->misscost, lookupcost + misscost);
            incrDRcount (thecache[i]->stats->misscount);
        } else {
            incrIcost (thecache[i]->stats->misscost, lookupcost + misscost);
            incrIcount (thecache[i]->stats->misscount);
        }
    }
}

CacheAssociativityT assocFindEmpty (CacheT* cache, AddressT address) {
    for (int i = 0; i < cache->associativity; i++) {
        if (!(status (cache->cachedata[i], address) & VALID))
          return i;
    }
    return cache->associativity; // miss
}

CacheAssociativityT assocFindVictim (CacheT* cache) {
    return random() & cache->assocmask; // only get here if all ways occupied
}

//////////////////////////////////// STATIC FUNCTIONS ////////////////////////////////////


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

void reportstats (CacheT *cache[]) {
    int N = countlevels (cache), // 1 more than highest index in cache array
        L1Iindex = 0;
    ELAPSED totaltime       = 0,
            totalhits       = 0,
            totalmisses     = 0,
            totalinclusions = 0,
            instructions = getIcount (cache[L1Iindex]->stats->hitcount) + 
                           getIcount (cache[L1Iindex]->stats->misscount);
    bool splitL1 = cache[0]->split;
    if (splitL1) {
        N++;
    }
#ifdef DEBUG
    fprintf(stderr, "#####MAX HITCOST %lu######\n", maxhitcost);
#endif
    int level = 1;
    printf ("level\tHits\tmisses\tincl.\thit t\tmiss t\n");
    for (int i = 0; i < N; i++) {
        ELAPSED misscost = 0, hitcost = 0,
            icost = 0,
            misscount = 0, hitcount = 0,
            icount = 0, inclusions = 0;
        misscost  += getIcount(cache[i]->stats->misscost);
        misscost  += getDRcount(cache[i]->stats->misscost);
        misscost  += getDWcount(cache[i]->stats->misscost);
        misscount += getIcount(cache[i]->stats->misscount);
        misscount += getDRcount(cache[i]->stats->misscount);
        misscount += getDWcount(cache[i]->stats->misscount);
        hitcost   += getIcount(cache[i]->stats->hitcost);
        hitcost   += getDRcount(cache[i]->stats->hitcost);
        hitcost   += getDWcount(cache[i]->stats->hitcost);
        hitcount  += getIcount(cache[i]->stats->hitcount);
        hitcount  += getDRcount(cache[i]->stats->hitcount);
        hitcount  += getDWcount(cache[i]->stats->hitcount);
        inclusions += getIcount(cache[i]->stats->inclusioncount);
        inclusions += getDRcount(cache[i]->stats->inclusioncount);
        inclusions += getDWcount(cache[i]->stats->inclusioncount);
        printf ("$[L%d%s]\t%lu\t%lu\t%lu\t%lu\t%lu\n",
            level, cache[0]->split?(i==0?"I":(i==1?"D":"")):"", hitcount, misscount, inclusions, hitcost, misscost);
        if (i > 0)
            level++;
        else if (!splitL1)
            level++;
        totaltime   += hitcost + misscost;
        totalhits   += hitcount;
        totalmisses += misscount;
        totalinclusions += inclusions;
  }
  printf ("Total elapsed time %lu, total hits %lu, total misses %lu, evictions for"
          " inclusion %lu; instructions: %lu\n",
          totaltime, totalhits, totalmisses, totalinclusions, instructions);
}

static AllStatsT* init_all_stats () {
    AllStatsT* allstats = malloc (sizeof (AllStatsT));
    allstats->hitcount = init_stats ();
    allstats->misscount = init_stats ();
    allstats->replacecount = init_stats ();
    allstats->inclusioncount = init_stats ();
    allstats->hitcost = init_stats ();
    allstats->misscost = init_stats ();
    return allstats;
}

static void deconstruct_all_stats (AllStatsT* stats) {
    deconstruct_stats(&(stats->hitcount));
    deconstruct_stats(&(stats->misscount));
    deconstruct_stats(&(stats->replacecount));
    deconstruct_stats(&(stats->inclusioncount));
    deconstruct_stats(&(stats->hitcost));
    deconstruct_stats(&(stats->misscost));
}

// write in a given level; in main memory, associativity is set to 0 so nothing happens
static void dowrite (CacheT *level, AddressT where) {
    for (int i = 0; i < level->associativity; i++) {
        if (rawCacheHit (level->cachedata[i], where)) {
            setbits (level->cachedata[i], blockaddress (level->cachedata[i], where),
                     MODIFIED);
            return;
        }
    }
}

#define BLOCKSIZE(mlcache,level) (getblocksize (mlcache[level]->cachedata[0]))

// from the top down, if this address hits: write back if necessary, modify level
// down and invalidate; based on numbering scheme, we need not worry about split
// I and D L1 etc. (a hit in one means it should not be in the other).
// If any levels below a given one have a bigger block size, need to remove
// all additional blocks not just the one containing the address of interest.
// Does not invalidate the level that triggered this: must fix up there.
static void maintaininclusion (CacheT* multilevelcache [], int misslevel, AddressT where,
                               ReftypeT reftype) {
    LatencyT writecosts = 0;
    BlocksizeT biggestbelow = BLOCKSIZE(multilevelcache,misslevel);
    for (int i = misslevel-1; i >=0; i--) {
        if (BLOCKSIZE(multilevelcache,i) > biggestbelow) {
            biggestbelow = BLOCKSIZE(multilevelcache,i);
        }
    }
    LatencyT maxlookupcost = multilevelcache[0]->lookupoverhead;
    for (int i = 0; i < misslevel; i++) {
       // CachesizeT whichblock = blockaddress (multilevelcache[i]->cachedata[0], where);
        BlocksizeT blocksize = BLOCKSIZE(multilevelcache,i),
                   blocks = 1;   // how many blocks to remove (>1 if bigger blocks below this level)
        AddressT place = where;
        // lookups in parallel, only account for biggest
        LatencyT lookupcost = multilevelcache[i]->lookupoverhead;
        if (lookupcost > maxlookupcost)
            maxlookupcost = lookupcost;
        if (biggestbelow > blocksize) {
            Bitshift offsetbits = calculateOffsetBits (biggestbelow);
            blocks = blocksize / biggestbelow;
            place = (place >> offsetbits) << offsetbits; // align address to biggest block below
        }

        for (int way = 0; way < multilevelcache[i]->associativity; way++) {
            RawCacheT *thisway = multilevelcache[i]->cachedata[way];
            for (int j = 0; j < blocks; j++) {
                if (rawCacheHit (thisway, place)) {
                    if (mustWriteback (thisway, place)) {
                        // this will work even if i+1 is DRAM layer
                        dowrite (multilevelcache[i+1], place);
                        // writecosts should increment here if any delay
                        // in practice we only really need to write to
                        // the level that incurred the miss in some cases
                        // e.g. if the block is being replaced but keep it simple
                    }
                    invalidate(thisway, place); // FIXME: does this trigger maintaininclusion for the extra blocks?
                    if (reftype == WRITE) {
                       incrDWcount (multilevelcache[i]->stats->inclusioncount);
                    } else if (reftype == READ) {
                       incrDRcount (multilevelcache[i]->stats->inclusioncount);
                    } else {
                       incrIcount (multilevelcache[i]->stats->inclusioncount);
                    }

                }
                place += blocksize;   // push into next block
            }
        }
    }
    // account for look up costs at the level that caused the miss
    if (reftype == FETCH)
        incrIcost (multilevelcache[misslevel]->stats->misscost, maxlookupcost);
    else if (reftype == READ)
        incrDRcost (multilevelcache[misslevel]->stats->misscost, maxlookupcost);
    else
        incrDWcost (multilevelcache[misslevel]->stats->misscost, maxlookupcost);

}

//////////////////////////////////// UNIT TEST DRIVER ////////////////////////////////////

#ifdef UNITTESTCACHETYPES

// FIXME: majorly wrong since splitting raw cache off to another file

// only compiled if compiler line has -DUNITTESTCACHETYPES -- needs to link with
// error.o and stringutils.o

// for each level:
// size in bytes, block size, hit time, lookup overhead, associativity, split
// split only implemented for L1
// 0 as associativity in main memory (only hit time used): must be lowest level
// NULL terminates the array
const char* twolevel [] = {
    "16384  32   1  1 1 0",
    "262144 32  10  2 2 0",
    "0       0 100  0 0 0",
    NULL
    },
    *twolevelsplit [] = {
        "16384  32   1 1 1 1",
        "16384  32   0 1 1 1",
        "262144 32  10 2 2 0",
        "0       0 100 0 0 0",
        NULL
    },
    *threelevel [] = {
        "16384   32   1 1 1 0",
        "262144  32  10 2 2 0",
        "4194304 64  30 5 2 0",
        "0        0 100 0 0 0",
        NULL
    },
    *threelevelsplit [] = {
        "16384   32   1 1 1 1",
        "16384   32   0 1 1 1",
        "262144  32  10 2 2 0",
        "4194304 64  30 5 2 0",
        "0        0 100 0 0 0",
        NULL
    };

const Trace tracerecord[] = {
    {READ, 1},
    {READ, 31},
    {READ, 524288},  // 16384*32 to wrap around and get the same place direct-mapped
    {READ, 1048576}, // wrap aroun again, 2-way must miss
    {WRITE, 128},
    {READ, 128},
    {FETCH, 256}
};
    
CacheT** createExample (const char *lines []) {
    CacheSetupT** setup = NULL;
    for (int i = 0; lines[i]; i++)
         setup = addCacheParameters (setup, makeCacheParametersStr((char*)(lines[i])));
    CacheT** example = initmultilevelcache (setup);
    deconstruct_setup (setup);
    return example;
}
   StatsT *hitcount,
          *misscount,
          *replacecount,
          *hitcost,
          *misscost;
ELAPSED getIcount (StatsT * stats);

ELAPSED getDRcount (StatsT * stats);

ELAPSED getDWcount (StatsT * stats);




int main () {
    CachesizeT nbytes = 4096*8, // 32KiB
      testinserts [] = {0, 1, 4096, 129537};  // byte address at which to test insertion
    int Ninserts = sizeof(testinserts)/sizeof(CachesizeT);
    int notraces = sizeof(tracerecord)/sizeof(Trace);
    fprintf(stderr,"inserting %d entries, %d traces\n", Ninserts, notraces);
    BlocksizeT blocksize = 32;
    RawCacheT * cache = initrawcache (nbytes/blocksize, blocksize);
    fprintf(stderr, "address mask for %u 0x%x\n", blocksize, getAddressMask (blocksize));
    fprintf(stderr, "blockshift for %u 0x%x\n", blocksize, calculateOffsetBits (blocksize));
    fprintf(stderr, "index mask for %u blocks = 0x%x\n", nbytes/blocksize, getIndexMask(nbytes/
            blocksize));
    fprintf(stderr, "######### done %u KiB cache #########\n", nbytes);
    for (int i = 0; i < Ninserts; i++)
        insert (cache, testinserts[i]);
    for (BlocksizeT blocksize = 32; blocksize < 1024; blocksize *= 2) {
        fprintf(stderr, "address mask for %u 0x%x\n", blocksize, getAddressMask (blocksize));
        fprintf(stderr, "blockshift for %u 0x%x\n", blocksize, calculateOffsetBits (blocksize));
        fprintf(stderr, "index mask for %u blocks = 0x%x\n", nbytes/blocksize, getIndexMask(nbytes/
                blocksize));
    }
    unsigned test = 1;
    while (true) {
        unsigned lasttest = test;
        test*=2;
        test &= getIndexMask(4);
        if (test <= lasttest) {
            fprintf(stderr, "%u biggest index mask <= %u, mask 0x%x ", lasttest, 8, getIndexMask(4));
            fprintf(stderr, "index bits %u, offset bits %u\n",  calculateIndexBits(4), calculateOffsetBits(8));
            break;
        }
    }
    fprintf(stderr, "Making 2 level\n");
    CacheT** twolevelCache = createExample (twolevel);
    fprintf(stderr, "Made 2 level\n");
    //cacheHit (CacheT* thecache[], AddressT where, ReftypeT reftype)
//     LatencyT misscost = 0, hitcost = 0,
//              icount = 0;
//      int foundat = findInCache (thecache, where, reftype), // 0 or 1 if no miss

    for (int i = 0; i < notraces; i++) {
        if (tracerecord[i].reftype != EXCEPTION) {
            handleReference (twolevelCache, tracerecord[i].addr, tracerecord[i].reftype);
            reportstats (twolevelCache);
        }
    }
    
}

#endif // UNITTESTCACHETYPES
