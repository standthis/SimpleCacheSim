/* stats.c
 *
 * Record and retrieve stats on memory accesses; numbers are separately
 * record for instruction fetches (I), data reads (DR) and data writes (DR).
 * Use a separate data structure for each type of stats needed (e.g., misses,
 * miss time, different caches).
 *
 * Author: Philip Machanick
 * Created: 22 March 2018
 * Updated: 23 May 2018
 *
 * Copyright (c) free to copy provided the author's attribution
 * is retained.
 *
 */

#include "stats.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>

struct Stats {
    ELAPSED
      instructions,
      datareads,
      datawrites;
};

struct MultilevelStats {
    StatsT * levelstats; // array of stats
    int      N;          // number of levels
};

typedef struct Stats StatsT;

const static StatsT init_stats_value = { 0 };

// call at start
StatsT * init_stats () {
    StatsT * newstats = malloc (sizeof(StatsT));
    *newstats = init_stats_value;
    return newstats;
}


// increase instruction count
void incrIcount (StatsT * stats) {
    stats->instructions++;
}

// increase data read count
void incrDRcount (StatsT * stats) {
    stats->datareads++;
}


// increase data write count
void incrDWcount (StatsT * stats) {
    stats->datawrites++;
}

// increase instruction cost
void incrIcost (StatsT * stats, ELAPSED cost) {
    stats->instructions += cost;
}

// increase data read cost
void incrDRcost (StatsT * stats, ELAPSED cost) {
    stats->datareads += cost;
}

// increase data write cost
void incrDWcost (StatsT * stats, ELAPSED cost) {
    stats->datawrites += cost;
}


ELAPSED getIcount (StatsT * stats) {
    return stats->instructions;
}

ELAPSED getDRcount (StatsT * stats) {
    return stats->datareads;
}


ELAPSED getDWcount (StatsT * stats) {
    return stats->datawrites;
}


// call at the end to dispose data structure
// need extra pointer layer to dispose and reset pointer to NULL
void deconstruct_stats (StatsT ** stats) {
    free (*stats);
    *stats = NULL;
}
