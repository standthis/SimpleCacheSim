/*
 * simulateMultilevelAssoc.h
 * 
 * Trace-driven N-way multilevel associative cache simulation, with stats.
 * Configuration is passed in and used to set up the simulated cache.
 * Resets stats after each trace file; would need a scheduler and other OS machinery
 * to simulate a real multitasking workload
 *
 * Philip Machanick
 * June 2018
 * 
 */

#ifndef simulateMultilevelAssoc_h
#define simulateMultilevelAssoc_h

#include "multilevelAssoc.h"

// report timing stats after simulating workload to completion
void simulateMultilevelAssoc (CacheSetupT* paremeters[]);

#endif // simulateMultilevelAssoc_h
