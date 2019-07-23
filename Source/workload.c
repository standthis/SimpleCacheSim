/* workload.c
 *
 * Types and operations for reading in a workload
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

#include <string.h>
#include <stdlib.h> // for malloc, free and exit
#include <stdbool.h>
#include "workload.h"
// reinstate this if you need an instrumented version of malloc
// #include "my_malloc.h" // breaks getline, since it calls malloc internally
#define my_malloc malloc
#define my_free free

// forward declarations so we can make pointers
typedef struct WorkloadStruct Workload;
typedef struct WorkloadListStruct WorkloadList;

// doubly-linked list of workloads
struct WorkloadListStruct {
  Workload *process;
  WorkloadList *previous, *next;
};

const int MAXNAME = 80; // geline can resize this
static PID MAXPID = 0;  // NB: add 1 for number of PIDs, since this is a zero-based index
// index by PID: since we read the workload list upfront we know
// how many processes there are
static WorkloadList **processes;

// when we create one of these, we open the file
// and close the file when deallocating
struct WorkloadStruct {
  char *filepath;
  char type;
  FILE *fp;
  bool more;         // true unless at EOF
  unsigned int addr; // not an address if an exception: wait time in instructions
};

static WorkloadList *workloads = NULL, *lastnew = NULL;

// report the type stored with a workload (single char)
char getType (PID proc) {
    return processes[proc]->process->type;
}

static Workload* initworkload (char* filename) {
  FILE *fp;
  Workload * newworkload;
  int len = strlen(filename);
  char type = ' ';
  if (filename[len-1] == '\n')
    filename[len-1] = '\0';
  // check now if the file name ends with *<type>; if so
  // strip that off and record the first char after '*' as type
  for (int i = 0; i < strlen(filename); i++)
    if (filename[i] == '*') {
       type = filename[i+1]; // worst this can be: '\0'
       filename[i] = '\0';                // adjust string end
       break;
    }
  if (!(fp=fopen(filename, "r"))) {
    fprintf(stderr,"ERROR: file `%s' can't be opened: ", filename);
    perror(NULL);
    return NULL;
  }
  newworkload = (Workload*) malloc (sizeof(Workload));
  // remember +1 for null terminator char:
  newworkload->filepath = malloc (sizeof(char)*(strlen(filename)+1));
  newworkload->fp = fp;
  newworkload->more = true;
  newworkload->type = type;
  strcpy (newworkload->filepath, filename);
  newworkload->addr = 0; // actually set each time we get a new address
  return newworkload;
}

static WorkloadList * initWLlist (Workload *element) {
  WorkloadList *newlist = (WorkloadList*) malloc (sizeof (WorkloadList));
  newlist->process = element;
  newlist->next = NULL;
  return newlist;
}

// if no terminating null char overwrite last position with one:
// we will break sooner or later anyway
static void fixmax (char *name, int max) {
  int i;
  for (i = 0; i < max; i++)
    if (name[i] == '\0')
      return;
  name[max-1] = '\0';
  fprintf(stderr,"WARNING: file path longer than %d, truncated to `%s'\n", max, name);
}

// set up an array indexed on PID so we can find a process easily
// to get the next workload element
static void init_procs ( WorkloadList * wl) {
  PID i;
  processes = malloc (sizeof (WorkloadList) * (MAXPID+1));
  for (i = 0; i <= MAXPID; i++) {
    processes[i] = wl;
    wl = wl->next;
  }
}

FILE *getfile (PID proc) {
  if (proc <= MAXPID) {
    if (processes[proc]->process->more)
       return processes[proc]->process->fp;
    else
       return NULL;
  } else {
    fprintf(stderr,"ERROR: attempt to access PID `%lu' > max= `%lu'\n", proc, MAXPID);
    return NULL;
  }
}

PID getmaxPID () {
  return MAXPID;
}

// read file paths from stdin and check each exists; if any don't return false
// temporarily obscure my definitions of malloc and free so getline works OK
bool init_workloads () {
#undef malloc
  char *line = malloc(sizeof(char)*(MAXNAME+2)); // al1ow 1 extra for each of \0 and \n 
#define malloc my_malloc
  size_t lineN = 0;
  bool goodfile = false, badfile = false;
  // testing for EOF OK here because we either read a word or don't
  // hint for format that limits length and can read anything but new line
  while (getline(&line, &lineN, stdin) != EOF) {
    //fixmax (line, MAXNAME);
    Workload * newworkload = initworkload (line);
    if (newworkload) { // if not we had a bad file path this time
      goodfile = true;
      WorkloadList * newelement = initWLlist (newworkload);
      newelement->previous = lastnew; // NULL if this is the first
      if (!workloads) {
        workloads = lastnew = newelement;
      } else {
        if (!lastnew) {
          fprintf (stderr, "ERROR: lastnew not set when it should be.\n");
          exit(1);
        }
        lastnew->next = newelement;
        lastnew = newelement;
        MAXPID++;
      }
    } else {
      badfile = true;
    }
  }
#undef free
if (line)
    free (line);
#define free my_free
  line = NULL;
  if (!goodfile) {
    fprintf (stderr, "no usable file paths, giving up.\n");
    return false;
  } else if (badfile)
    fprintf (stderr, "at least one bad file path, carrying on...\n");
  init_procs (workloads);
  return true;
}

void filedone (PID proc) {
  processes[proc]->process->more = false;
}

// call at termintation
void deconstruct_workload () {
  WorkloadList * wl = workloads, *wl_prev=NULL;
  free (processes);
  while (wl) {
  	wl_prev = wl;
  	wl = wl->next;
  	fclose(wl_prev->process->fp);
  	free (wl_prev->process->filepath);
  	free (wl_prev->process);
  	free (wl_prev);
  }
}

