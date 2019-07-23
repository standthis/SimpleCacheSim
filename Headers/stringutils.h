/* stringutils.h
 *
 * Various string utilities including converting a buffer containing a single
 * string with line breaks to an array of strings (in place -- the original buffer
 * has line breaks replaced by null terminators, '\0', and a new array points
 * to line starts).
 *
 * Author: Philip Machanick
 * Created: May 2015
 * Updated: June 2018
 *
 * Copyright (c) free to copy provided the author's attribution
 * is retained.
 *
 */

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <stdbool.h>

// convert a single string into multiple strings, replacing
// each line break '\n' by '\0' and return an array of char*
// pointing to each line start; last element of lines array
// is NULL to signify the end so we don't need to kee track
// of the length
char **linify (char buffer[]);

// dispose of a lines array: dispose of the underlying char array
// then the outer container; if you do not need to dispose both
// e.g. the char data is not allocated with malloc, just call free
// on the outer pointer
void dispose_lines(char **lines);

// return number of lines in a lines array created by linify
// relying on lines[Nlines] == NULL
int lineslen (char ** lines);

// simple check that a line only contains digits and whitespace
// will return false if a minus sign is present
bool isnumbers (char line[]);

// concatenate two strings, returning the result
// in newly allocated memory; either or both can
// be NULL; if either is NULL, return a copy of the other;
// if both NULL, return NULL
char *mystrcat (char *a, char *b);

// compare two lines using string comparison:
// result < 0 if a < b
// result = 0 if a == b
// otherwise return > 0 result
int linecomp(const char **a, const char **b);

// read a line of text, and allocate space for it. Stop on EOF or line break.
// returns NULL on EOF
char * readaline ();

// convenient type sortcomp to cast to type required for library sorting
typedef int (*sortcomp) (const void *, const void *);

#endif // STRINGUTILS_H
