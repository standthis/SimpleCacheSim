/* 
 * readtrace.c
 *
 * Types and operations for reading in a trace file
 * in format [IWRX] hex number where:
 * I means instruction reference (never modified)
 * W means data write (memory modified at that address)
 * R means data read (that address accessed but unchanged)
 * X exception, resulting in wait time of the given number of instructions
 *
 * terminates either on a line starting with # or on EOF
 * does not check contents
 *
 * Author: Philip Machanick
 * Created: 5 January 2012
 * Exceptions added: 8 March 2013
 *
 * Copyright (c) free to copy provided the author's attribution
 * is retained.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "readtrace.h"
#include "workload.h"

typedef enum {invalid, unused, used} Tracestates;

typedef struct {
	FILE *tracefile;
	Trace record;
	Tracestates validity;
} Traceinfo;

static Traceinfo *tracestate = NULL;

void init_tracing (PID maxpid) {
  int i;
  tracestate = malloc (sizeof (Traceinfo) * (maxpid+1));
  for (i = 0; i < (maxpid+1); i++) {
  	tracestate[i].tracefile = getfile(i);
  	tracestate[i].record.reftype = EOFSYMBOL;
  	tracestate[i].validity = invalid;
  }
}

// go back to previous: if none read before, the trace state remains invald
// only remember one previous, on the basis that we can interrupt one memory
// reference at a time (if an IF, the data references wouldn't happen on a
// real machine if the instruction causing them faulted in a recoverable way;
// a real machine would more typically restart an instruction than a data reference
// on an interrupt)
void backtrack (PID p) {
  if (tracestate[p].validity != invalid)
    tracestate[p].validity = unused;
}

// the next record in the trace file or if the last is unused, return that instead
// in all cases mark the last reference as used (can be undone by backtrack())
Trace next_addr (PID proc) {
  if (!tracestate)
    init_tracing (getmaxPID ());
  if (tracestate[proc].validity != unused) {
    int resultcount = fscanf(tracestate[proc].tracefile, "%c %x\n", &(tracestate[proc].record.reftype), &(tracestate[proc].record.addr));
    if (resultcount < 2) {
      tracestate[proc].record.reftype = EOFSYMBOL;
    }    
  }
  tracestate[proc].validity = used;
  return tracestate[proc].record;
}

// get rid of the local array and set its pointer MULL to be safe
void deconstruct_tracing () {
  free (tracestate);
  tracestate = NULL;
}
