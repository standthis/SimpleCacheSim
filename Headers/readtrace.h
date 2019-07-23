/*
 * readtrace.h
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

#ifndef readtrace_h
#define readtrace_h

#include "generaltypes.h"

typedef char ReftypeT;

typedef struct {
  ReftypeT reftype;      // I, W, R, or X: read ends on EOF or #
  unsigned int addr; // not an address for exceptions: wait time in instructions
} Trace;

// stupid C compiler allocates storage more than
// once for const char so #define instead
#define EOFSYMBOL '#'
#define READ 'R'
#define WRITE 'W'
#define FETCH 'I'
#define EXCEPTION 'X'

// set up internal data structures: must be called first
void init_tracing (PID maxpid);
// return the next entry from the trace file (or previous if you backtracked)
Trace next_addr (PID proc);
// restore the previously read trace as the next to read
void backtrack (PID p);
// dipose internal data structures: call after all traces completed
void deconstruct_tracing ();

#endif // readtrace_h
