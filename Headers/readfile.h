/*
 * readfile.h
 *
 * Read in a whole file and report its length with the option to add a null
 * terminator '\0' at the end.
 *
 * Author: Philip Machanick
 * Created: May 2015
 *
 * Copyright (c) free to copy provided the author's attribution
 * is retained.
 *
 */

// readfile.h

#ifndef READFILE_H
#define READFILE_H

#include <stdbool.h>
#include <stdio.h>  // for FILE declaration

// read a whole file and return newly allocated member
// of size stored in length, from the named file path
// If string data and a null terminator is required,
// isstring should be true; if the buffer is NULL, allocate it
char * read_file (char name[], long *length, bool isstring, char *buffer,
       long offset);  // offset 0 to read whole file

// as above, but opened already; does not close
char * read_file_fptr (FILE *f, long *length, bool isstring, char * buffer,
       long offset);  // offset 0 to read whole file

#endif // READFILE_H
