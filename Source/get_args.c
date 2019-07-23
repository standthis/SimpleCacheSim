/* get_args.c
 *
 * Check command line and print usage if wrong; if right
 * open config file, read it in and convert to array
 * of strings, one per line.
 *
 * Author:   Philip Machanick
 * Created:  11 April 2012
 * Modified: 21 March 2018, 30 May 2018
 *
 * Copyright (c) free to copy provided the author's attribution
 * is retained.
*/

#include "get_args.h"
#include "stringutils.h"
#include "readfile.h"

#include <string.h>
#include <stdlib.h>


static const char pathseparator = '/';

static const char* usage = "USAGE: %s configfilename\n"
  "       reads a trace file simulating a cache, counting hits and misses.\n"
  "       If more than one trace file is given, the cache is flushed but\n"
  "       counts are not reset.\n"
  "       The configuration file should specify cache levels as follows.\n"
  "         size in bytes\n"
  "         block size\n"
  "         hit cost\n"
  "         lookup cost (added to miss cost)\n"
  "         associativity\n"
  "         split (1 for true 0 for not)\n"
  "       The split parameter only applies to the first level: if 1 in\n"
  "       the first entry that is taken as the L1I cache, the next as L1D.\n"
  "       All sizes must be powers of 2 >= 1 and costs >= 0; lower level\n"
  "       cache sizes must be >= upper levels to support multilevel inclusion.\n"
  "       Except: DRAM specified as all zeros excewpt hit time.\n"
  "input:\n"
  "       list of trace files (stdin)\n"
  "output (stdout: X, Y and Z are calculated counts):\n"
  "      processed X memory references, Y misses; Z hits\n"
;

static void display_usage (char *progname, int die) {
  char *name_nopath = &progname[strlen(progname)-1];
  while (name_nopath > progname) {
    if (*name_nopath == pathseparator) {
      name_nopath ++;
      break;
    }
    name_nopath --;
  }
  fprintf(stderr, usage, name_nopath);
  if (die) {
    fprintf(stderr,"dying with %d\n", die);
    exit(die);
  }
}

char** get_args (int argc, char *argv[]) {
  unsigned int namelength;
  int i;
  long filelength = 0;
  char * buffer;
  char ** lines;
  if (argc != 2) {
    fprintf(stderr,"bad argc = %d\n", argc);
    display_usage(argv[0], -1);
  }
  FILE *infile = fopen (argv[1], "r");
  if (!infile) {
    perror ("failed to open configuration file");
    return NULL;
  }

  if (!infile) {
    fprintf(stderr,"Cannot open configuration file `%s'\n", argv[1]);
    display_usage(argv[0], -1);
  }
  // true: null-terminated string, NULL: memory allocated, 0: whole file from start
  buffer = read_file_fptr (infile, &filelength, true, NULL, 0);
  if (fclose(infile) != 0) {               // close file: return 0 == success
     perror ("failed to close configuration file");
  }
  if (buffer)
      return linify(buffer);
  else
      return NULL;
}
