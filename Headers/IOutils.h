/* IOutils.h
 * 
 * Open a file for reading or writing and find out its length.
 *
 * Philip Machanick
 * June 2018
 *
 */

#ifndef IOUTILS_H
#define IOUTILS_H

#include <stdio.h>

FILE *openWrite (char name[], char * extension);
FILE *openRead (char name[], char * extension);
long get_length (char name[]);
long get_length_fptr (FILE *f);

#endif // IOUTILS_H
