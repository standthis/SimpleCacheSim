/* stats.h
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

#ifndef stats_h
#define stats_h

#include <stdbool.h>

#include "generaltypes.h" // for type ELAPSED

// make 1 of these for each kind of stats you want including clock
// ticks for each tye pof reference; use a separate one for hits
// and misses, for example
typedef struct Stats StatsT;

// call at start
StatsT* init_stats ();

// increase instruction count
void incrIcount (StatsT * stats);

// increase data read count
void incrDRcount (StatsT * stats);


// increase data write count
void incrDWcount (StatsT * stats);

// increase instruction cost
void incrIcost (StatsT * stats, ELAPSED cost);

// increase data read cost
void incrDRcost (StatsT * stats, ELAPSED cost);

// increase data write cost
void incrDWcost (StatsT * stats, ELAPSED cost);


// get values of counters
ELAPSED getIcount (StatsT * stats);

ELAPSED getDRcount (StatsT * stats);

ELAPSED getDWcount (StatsT * stats);

// call at the end to dispose data structure (for each one used)
// need extra pointer layer to dispose and reset pointer to NULL
void deconstruct_stats (StatsT ** stats);

#endif // stats_h
