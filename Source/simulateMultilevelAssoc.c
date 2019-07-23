/*
 * simulateSimpleTimedAssocitative.c
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

#include <stdlib.h> // random and malloc
#include <stdio.h>

#include "simulateMultilevelAssoc.h"
#include "workload.h"
#include "error.h"
#include "multilevelAssoc.h"
 
void simulateMultilevelAssoc (CacheSetupT* paremeters[]) {
  Trace tracerecord;
  PID pid, maxPID = getmaxPID ();
  
  srandom (1); // for repeatability: this is the default initialization of random
  for (pid = 0; pid <= maxPID; pid++) {
     CacheT** cache = initmultilevelcache (paremeters);
     int Nlevels = countlevels (cache);
     printf ("workoad [%lu], %d levels\n", pid, Nlevels);
     while (true) {
         tracerecord = next_addr (pid);
         if (tracerecord.reftype == EXCEPTION)
             continue; // skip these
         if (tracerecord.reftype != READ &&
             tracerecord.reftype != WRITE &&
             tracerecord.reftype != FETCH) {
             // done with this cache -- deallocate it 
             break; // switch to next PID
             }  // next trace record
             
             // cacheHit (CacheT* thecache[], AddressT where, ReftypeT reftype
             handleReference (cache, tracerecord.addr, tracerecord.reftype);
    }
    reportstats (cache);
    deconstruct_multilevelcache (cache);
  }
  deconstruct_tracing ();
}

