/* workload.h
 *
 * types and operations for reading in a workload
 * with one file path per line representing files containing traces
 * Each line is a different trace file representing a different
 * process.
 * 
 * terminates on EOF
 * 
 * Author: Philip Machanick
 * Created: 9 March 2013
 * 
 * Copyright (c) free to copy provided the author's attribution
 * is retained.

 */

#ifndef workload_h
#define workload_h

#include <stdbool.h>
#include "generaltypes.h"
#include "readtrace.h"
#include <stdio.h> // for type FILE

// call at start
bool init_workloads ();

// get highest PID (add 1 for number of processes)
PID getmaxPID ();

// get the process type for a given PID
char getType (PID proc);

// get file handle for a given PID
FILE *getfile (PID proc);

// indicate that the given file has hit EOF
void filedone (PID proc);

// call at the end to dispose data structures
void deconstruct_workload ();

#endif // workload_h
