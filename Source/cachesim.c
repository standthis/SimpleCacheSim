/* Simulate a multilevel cache: top level can bs split I and D
 * All levels can be asscociative; assciativity is set to 1 if not.
 * Caches are indexed from 0:
 * -- top level: 0 if not split, 0 for I and 1 for D
 * -- associativity: implemented by duplicating the data structures but
 *    at the same actual cache has the same index (including split caches)
 * Caches are built on a simple DM cache (implemented in rawcache.c) that has
 * no timing; associativity is implemented by an array of DM caches and a hierarchy
 * by an array of these arrays.
 * DRAM is not modelled except its access time -- if there is no hit in lowest-
 * level cache (LLC) a DRAM access always succeeds (infinite DRAM model).
 *
 * Inclusion is mantained: if a block is invalidate for any reason including
 * victim of replacement, it is first invalidated in the top-level cache
 * (I and D both checked if split) in case it must be written back.
 *
 * Timing model: for simplicity assume infinite writeback buffer and all
 * writebacks are complete before the same block is needed. Also assume no
 * misses from DRAM and that all misses result in a stall. All cache access
 * have minimal tag lookup latency that is not counted in a hit on L1 but is
 * counted for finding victims and maintaining inclusion. Miss cost is usually
 * this latency plus cost of accessing next level down.
 *
 * Philip Machanick
 * June 2018
 * 
 */

#include "get_args.h"
#include "simulateMultilevelAssoc.h"
#include "workload.h"
#include "error.h"


int main (int argc, char *argv []) {
    // check command line and get config file ready to read
    char** configlines = get_args (argc, argv);
    // if good, read the workload file list (from stdin)
    if (!configlines)
       error (configFileError, false, "Configuration file missing or not openable",
              __LINE__, __FILE__);
    if (!init_workloads ())
       error (workloadError, false, "No usable workload files", __LINE__, __FILE__);
    // initialize main data structures
    CacheSetupT** parameters = 
    // read cache config from file name in command line
        getconfig (configlines);
    reportParameters (parameters);
    
    // run simulation
    simulateMultilevelAssoc (parameters);
    // report stats
    // deconstruct data structures
    deconstruct_setup (parameters);
    deconstruct_workload ();
}
