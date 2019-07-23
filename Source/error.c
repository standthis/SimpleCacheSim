/* error.c
 *
 * Basic error processing – print a message containing a passed in
 * string to augment the message based on the error code; optionally
 * terminate the program.
 * 
 * Philip Machanick
 * June 2018
 *
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h> // for exit

// idea from https://stackoverflow.com/questions/21023605/initialize-array-of-strings
const char * errorstrings [] = {
  "Invalid number of blocks",
  "Invalid cache size",
   "Invalid block index",
   "Cache ID not next free number",
   "Associativity must be a power of 2 >=1",
   "Chosen victim can't be invalid",
   "Improperly formatted cache configuration line",
   "Unable to find or open cache configuration line",
   "Unable to open workload file",
   "Invalid number of levels setting up stats"
};

#define Nerrors (sizeof (errorstrings) / sizeof (const char *))

void error (unsigned errorcode, bool noexit, char* text,
            unsigned linenumber, char* filename) {
   fprintf (stderr, "ERROR");
   if (errorcode >= Nerrors)
       fprintf (stderr, " Error code `%u' out of range, max = %lu ", errorcode, Nerrors);
   else
       fprintf (stderr, " %s", errorstrings[errorcode]);
   if (text)
       fprintf (stderr, " %s", text);
   if (linenumber)
      fprintf (stderr, " at line %u", linenumber);
   if (filename)
      fprintf (stderr, " in source file `%s'", filename);
   fprintf (stderr, "\n");
   if (!noexit)
     exit (errorcode);
   else
       fprintf (stderr, "Continuing...\n");

}

#ifdef UNITTESTERROR

int main () {
    char message [] = "`some text'";
	for (unsigned i = 0; i <= Nerrors; i++) {
	  error (i, true, (i % 2 ? message : NULL),
	        (i % 2 ? __LINE__ : 0),
	        (i % 2 ? __FILE__ : NULL) );
	}
}

#endif // UNITTESTERROR