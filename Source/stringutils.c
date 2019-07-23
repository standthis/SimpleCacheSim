/* stringutils.c
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

#include <stdio.h> // for file IO
#include <stdlib.h> // for exit, malloc
#include <string.h>
#include <ctype.h>

#include "stringutils.h"


// granularity for allocating the string buffer
#define CHUNK 10


// convert a single string into multiple strings, replacing
// each line break '\n' by '\0' and return an array of char*
// pointing to each line start; last element of lines array
// is NULL to signify the end so we don't need to kee track
// of the length; element [0] == NULL signifies no lines
char **linify (char buffer[]) {
   int Nlines = 0, currentLine = 0;
   char ** lines = NULL;
   for (int i = 0; buffer [i] != '\0'; i++) {
     if (buffer[i] == '\n') {
       Nlines ++;
     }
   }
   lines = malloc ((Nlines+1) * sizeof (char*));
   lines [Nlines] = NULL; // end marker
   if (Nlines) {
      int currentLine = 0;
      lines[currentLine] = buffer;
      currentLine ++;
      // no point continuing if we have all the lines but check for '\0' to be safe
      for (int i = 1; buffer[i] != '\0'; i++) {
         if (buffer[i] == '\n') {
            buffer[i] = '\0';
            if (currentLine < Nlines) {
              lines [currentLine] = &(buffer[i+1]);
              currentLine++;
            }
         }
      }
   }
   return lines;
}

void dispose_lines(char **lines) {
    if (lines) {
        // dispose of the underlying char array, pointed at by first element of configlines
        free (lines[0]);
        // now free outer container
        free (lines);
    }
}

bool isnumbers (char line[]) {
    for (char *current = line; *current; current++)
        if (!(isdigit(*current)||isspace(*current)))
            return false;
    return true;
}

// count lines in a lines array as created by linify
int lineslen (char ** lines) {
   int i;
   // relies on line [Nlines] being NULL
   for (i = 0; lines[i]; i++) ;
   return i;
}

// compare two lines using string comparison:
// result < 0 if a < b
// result = 0 if a == b
// otherwise return > 0 result
int linecomp(const char **a, const char **b) {
    return strcmp (*a, *b);
}

// concatenate two strings, returning the result
// in newly allocated memory; either or both can
// be NULL; if either is NULL, return a copy of the other;
// if both NULL, return NULL
char* mystrcat (char *a, char *b) {
  int wholelength = 0;
  char *result = NULL;
  if (!a && !b)
    return NULL;
  if (!a)
    a = "";
  if (!b)
    b = "";
  wholelength = strlen (a) + strlen (b);
  result = malloc ((wholelength + 1) *sizeof (char));
  strcpy (result, a);
  strcat (result, b);
  return result;
}

// read a line of text ending on a line break or EOF; returns NULL on EOF
char * readaline () {
  unsigned buffer_size = CHUNK;
  char *buffer = malloc(CHUNK*sizeof(char)),
       *current;
  current = buffer;

  current[0] = '\0';
  while (fgets(current, CHUNK, stdin)) {
    if (buffer[strlen(buffer)-1] == '\n') { //isspace() or  == '\n'?
      buffer[strlen(buffer)-1] = '\0'; // found end of line, stop
      return buffer;
    }
    buffer = realloc (buffer, (buffer_size+CHUNK)*sizeof(char));
    current = &buffer[strlen(buffer)]; // start at last null-terminator
    buffer_size += CHUNK;
  }
  if (!strlen(buffer) && feof(stdin)) {
    free (buffer);
    return NULL;
  }
  return buffer;
}

