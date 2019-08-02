Multilevel caches are implemented using the following data structures in
multilevelAssoc.c: 
* Cache (CacheT)
* Multilevelcache
* Stats

The multilevelcache is implemented using an array of pointers to Cache objects. Cache objects are structs with properties such as cachedata (raw block and address information), latency, split and stats (results such as hit and miss count and cost, replacement count and inclusion count). These properties are used to store measurements relating to performance such as hit time and lookup overhead which determine the latency in which operations take.

Each cache in the array represents a different level of cache. This
representation is used to calculate points for each miss of a required
instruction or data where lower levels have higher penalties. 

A split cache is one where the level1 cache has been split into instructions and
data sections which is done for improved performance. In multilevelAssoc.c the
split and data cache are implemented using the same Cache struct as the other
levels. If split then the L1 cache is divided into instructions and data which
allow for simultaneous access of each within a pipeline, which can result in
improved performance on relevant workloads.
