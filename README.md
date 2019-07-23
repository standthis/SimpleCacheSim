# SimpleCacheSime
Simple cache simulator for education: basic multilevel cache simulator with
simplifying assumptions to aid pedegogy and to simplify implementation.

Philip Machanick
June 2018

To get started
==============
Build the code using make.

You should now be able to run it using:
$ ./cachesim Data/L3-unified-2way.conf < Data/test.workload 

This will run an example configured as:
* unified L1 totalling 32KiB, direct-mapped, divided into 32B blocks
* 256KiB (262144B) L2, 2-way associative, 32B blocks
* 4MiB (4194304B) L3, 2-way associative, 64B blocks
* DRAM: no misses just a cost to access
Run it and check its outputs and compare against the configuration file.

To convert to a split (instruction and data) L1, repeat the first line,
divide the size by 2 and change the last number from 0 to 1 (signifying
a split cache).

To make examples comparable, if you double block size, double the access time
of the level below -- except DRAM, where there is an initial high overhead to
set up an access. For example, use an overhead of 80 time units before data
moves and 20 time units per 32 bytes accessed. So a 32B LLC block has DRAM
access time of 100 units; for a 64B LLC block, 120 time units.

What follows is a description of data used, data structures and main functions
(in the order they are called starting from main).

At the end is a full list of source (C) ahd header (H) files.

DATA
====
A simulation is configured with a file that contains the following on each
line:

\<bytes\> <block size> <hit time> <tag check time> <associativity> <split>

defined as
* <bytes> -- total size of the cache
* <block size> -- block size in bytes
* <hit time>   -- time for a hit (0 for L1 data accesses)
* <tag check time> -- time to look up tags (not used in hits)
* <associativity> -- 1 for direct-mapped (DM); a power of 2 <=
  <bytes>/<block size>
* <split> -- 0 for unified data (D) and instruction (I), 1 for split;
  ignored except in L1

Trace files are structured as:
<type> <hex number>
where <type> is one of:
* I -- instruction fetch
* R -- data read
* W -- data write
* X -- exception
The file ends at end of file or if ``#eof'' is read.

For the DRAM layer, all numbers are 0 except the hit time, used to cost
lowest-level cache (LLC) misses. Infinite DRAM is modelled, i.e., no misses
from DRAM.

DATA STRUCTURES
===============
Strategy: details of struct types is hidden in the C file that implements them;
they can only be accessed in other files through a pointer or a function in the
C file that defines the type. A typedef names struct types for convenience.

Defined in cachesetup.c:
-----------------------
Cache parameters are stored in a struct CacheSetup (typedef: CacheSetupT),
containing:
* totalblocks, blocksize, hittime, lookupoverhead, associativity split
All are as described in the configuration file (except the 0 or 1 value of
split is stored as type bool).

Defined in cachetypes.c:
-----------------------
A cache is represented as an array of data structures, one per level -- except
with a split L1, in which case the first element of the array is the L1I
(instruction) cache and the second the L1D (data) cache. The lowest level of
the array should be the DRAM layer. Each level is represented as struct Cache
(typedef CacheT).

Each level contains:
* an array of arrays of RawCacheT
- each array at one level represents a way of an N-way associative cache
* latencies for hit time and lookup overhead;
* associativity
* whether split
* assocmask used in associativity calculations
* array of stats (AllStatsT)

The struct AllStats (typedef AllStatsT) contains StatsT values for
* hitcount, misscount, replacecount, hitcost, misscost (all as pointers since
  the StatsT type is defined elsewhere)

Defined in rawcachetypes.c:
--------------------------
struct RawCache (RawCacheT)
* Nblocks (number of blocks in this cache structure)
* blocksize in bytes
* array of blocks (each type CacheBlockT)
* addressmask and indexmask (for extracting component of an address)
* offsetbits and indexbits (number of bits for address components)
A raw cache is direct-mapped (DM) and can be used as a building block. To
make an N-way associative cache, implement it as N raw caches, each 1/N of
the required size. It does not implement any timing as it is used in a
simplistic way and actual timing should be based on how it is used.

struct Cacheblock (typedef CacheblockT) contains the tags and enough of the
address to identify the block uniquely.

Defined in stats.c:
------------------
struct Stats (typedef StatsT) keeps track of stats for instructions, data
reads and data writes.

MAIN FUNCTIONS
==============
Files are listed in the order of first function call, starting from main.

cachesim.c
----------
The simulation starts from the main program:
* checks the command line (should give the configuration file name)
* checks that there is at least one usable file name in the workload file
  (read from stdin: redirect a file name on the command line if needed)
* creates a parameter data structure containing the configuration
* calls simulateMultilevelAssoc with the parameters to do the simulation
- if there is more than one trace file in the workload, each is run as
  to completion as a separate process and reported separately
* deallocates the parameters and workload data structures

cachesetup.c
------------
Sets up parameters in a data structure suitable for intializing a
multilevel associative cache. This is the glue between the main program
and the actual simulation. It contains functions to get the command line,
check the parameter file and turn the parameters into a format that can easily
be used to initialize.

simulateMultilevelAssoc.c
-------------------------
* uses the supplied parameters to configure a multilevel cache with the
  levels given in the configuration file, associativity and timing for
  each recorded in a single data structure
* for each trace file:
- reads each trace, discarding "X" for exception lines, and passes it to
  handleReference

multilevelAssoc.c
-----------------
The main implementation of a multilevel associative cache.
* handleReference checks for a hit and if so updates stats; if not calls
  handleMiss
* handleMiss finds the level at which the block is found (if not in LLC,
  ``finds'' it in DRAM), works out whether it must replace anything in layers
  above that and also in the event of a replacement, calls maintaininclusion
  to ensure that multilevel inclusion is maintained.

rawcache.c
----------
This file implements various utility functions for basic operations on a cache
in addition to the following:
* rawCacheHit checks if there is a hit in a DM cache (possibly a way in an
  associative cache)
* insert adds a block by setting its tag as valid and storing the address bits
* mustWriteback returns true if block should be written back before replacement

SOURCE FILES
============
IOutils.c                 -- open a file, find out its size
cachesetup.c              -- create and access cache parameters
cachesim.c                -- main program: sets up, launches,ends simulation
error.c                   -- reports and handles errors (option to exit)
get_args.c                -- config file from command line; opens and reads it 
multilevelAssoc.c         -- implements associative multilevel cache simulation
rawcache.c                -- implements a single DM cache with no timing
readfile.c                -- read file into buffer as a '\0'-terminated string
readtrace.c               -- read next line from the trace file
simulateMultilevelAssoc.c -- pass non-exception trace records to simulator
stats.c                   -- keep track of fetch, read, write stats in struct
stringutils.c             -- turn buffer of lines into strings array per line
workload.c                -- manage a list of trace files

HEADER FILES
============
All provide interfaces to the source files, except for generaltypes.h:
IOutils.h
cachesetup.h
error.h
generaltypes.h            -- names for widely-used types like sizes, counters
get_args.h
multilevelAssoc.h
rawcache.h
readfile.h
readtrace.h
simulateMultilevelAssoc.h
stats.h
stringutils.h
workload.h
