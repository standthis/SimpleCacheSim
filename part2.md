# WRITEUP 
In order to simulate variations in design trade-offs, a number of custom conf
files were created, namely:
Data/L3-unified-2way.conf
Data/l1.conf
Data/l1-large.conf
Data/large-cache.conf
Data/small-cache.conf
Data/split.conf

The results of the simulations are shown below. The trade-offs under
consideration include cache size, split vs. unified L1, number cache size, split vs. unified L1, number of levels and design trade-offs like a bigger cache vs. a smaller but faster cache. 
The aim of this experiment is to analyse the effects of these different design 
trade-offs on performance, namely the total elapsed time as this indicates 
improved performance under the given synthetic workload.

#Results 
The L3-unified-2way configuration provides a total elapse time of 3168413 and
total hits 961706 with total misses 28830. Whereas the same configuration with a
L1 split (block size split into data and instructions) provided elapsed time 6217051, total hits 961706, total misses 14811. This indicates a slow down of 3048638 with the addition of a split L1 cache given the same workload. A split design allows for more aggressive pipelining as it allows for data and instruction accesses to occur in parallel thereby improving performance. In this case this performance improvement was not seen as the simulation simply reduces the size of L1 and accesses only the second part of the split thereby resulting in a smaller L1 cache which explains the higher total miss rate associated with the split simulation.  


#L3-unified-2way.conf
	blks	blksize	hitT	lookupT	assoc	split?	Total Bytes
L1:	1024	32	1	1	1	0	32768
L2:	8192	32	10	2	2	0	262144
L3:	65536	64	30	5	2	0	4194304
DRAM:			120
workoad [0], 3 levels
level	Hits	misses	incl.	hit t	miss t
$[L1]	954019	14082	61	954019	1093575
$[L2]	5729	8353	0	0	313370
$[L3]	1958	6395	0	0	807449
Total elapsed time 3168413, total hits 961706, total misses 28830, evictions for inclusion 61; instructions: 598247

# split.conf
	blks	blksize	hitT	lookupT	assoc	split?	Total Bytes
L1I:	1024	32	1	1	1	1	32768
L1D:	8192	32	10	2	2	0	262144
L2:	65536	64	30	5	2	0	4194304
DRAM:			120
workoad [0], 2 levels
level	Hits	misses	incl.	hit t	miss t
$[L1I]	596945	1302	0	596945	18772
$[L1D]	362740	7114	0	3627400	1166485
$[L2]	2021	6395	0	0	807449
Total elapsed time 6217051, total hits 961706, total misses 14811, evictions for inclusion 0; instructions: 598247

# l1.conf
	blks	blksize	hitT	lookupT	assoc	split?	Total Bytes
L1:	131072	32	1	1	1	0	4194304
L2:	8192	32	0	0	2	0	262144
L3:	65536	64	0	0	2	0	4194304
DRAM:			120
workoad [0], 3 levels
level	Hits	misses	incl.	hit t	miss t
$[L1]	959742	8359	4315	959742	784118
$[L2]	0	8359	0	0	4315
$[L3]	1964	6395	0	0	771437
Total elapsed time 2519612, total hits 961706, total misses 23113, evictions for inclusion 4315; instructions: 598247
    
* Bigger vs smaller cache
Simulations were conducted to determine whether a larger vs smaller cache is more
performant under the given workload. The results show the large cache as having
Total elapsed time 3032099, total hits 961706, total misses 23339 and the
smaller cache as having Total elapsed time 3432356, total hits 961665, total misses 35410. This demonstrates a 400257 or a 12% slow down for the small cache when compared to the larger cache. An explanation for this is that having relatively smaller caches at different levels increasing the likelihood of miss at each level due to more a more limited blocksize. On the other hand having a smaller cache given a different workload can improve performance if program execution is more sequential in nature, thereby reducing misses due to more predictable instruction and data fetches. 

# large-cache
	blks	blksize	hitT	lookupT	assoc	split?	Total Bytes
L1:	8192	32	1	1	1	0	262144
L2:	131072	32	10	2	2	0	4194304
L3:	131072	64	30	5	2	0	8388608
DRAM:			120
workoad [0], 3 levels
level	Hits	misses	incl.	hit t	miss t
$[L1]	959455	8646	0	959455	954260
$[L2]	348	8298	0	0	311063
$[L3]	1903	6395	0	0	807321
Total elapsed time 3032099, total hits 961706, total misses 23339, evictions for inclusion 0; instructions: 598247

# small-cache
	blks	blksize	hitT	lookupT	assoc	split?	Total Bytes
L1:	256	32	1	1	1	0	8192
L2:	1024	32	10	2	2	0	32768
L3:	4096	64	30	5	2	0	262144
DRAM:			120
workoad [0], 3 levels
level	Hits	misses	incl.	hit t	miss t
$[L1]	950360	17741	3977	950360	1243192
$[L2]	6508	11233	0	0	425830
$[L3]	4797	6436	0	0	812974
Total elapsed time 3432356, total hits 961665, total misses 35410, evictions for inclusion 3977; instructions: 598247
