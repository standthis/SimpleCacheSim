/*
 * readfile.c
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

#include <stdio.h>  // for file IO
#include <stdlib.h> // for exit, malloc

#include "readfile.h"
#include "IOutils.h"

// read a whole file into a buffer and return in newly allocated
// memory of type char*
// parametersL
//   char name[]   file path
//   long *length  place to store bytes read
//   bool isstring true if a null terminator to be added
//   long offset: how far from the start (0 to read whole file)
// though the return value is typed as  char*, values read can be
// any type hence the need to signal whether a string
// ***deallocation of the returned buffer up to the caller***
char * read_file (char name[], long *length, bool isstring, char * buffer,
       long offset) {
  FILE *f = fopen(name, "rb"); // open to read; 'b' for legacy compatibility
  if (f) {                         // is it a good file?
    buffer = read_file_fptr (f, length, isstring, buffer, offset);
  } else {                     // No. Bad file, no biscuit.
      perror ("failed to open file");
      return NULL;
  }
  if (fclose(f) != 0) {               // close file: return 0 == success
     perror ("failed to close file");
  }
  // at this point if the file failed to close, hope data read is OK
  return buffer;
}

// as above but opened already and does not close; does the actual work
char * read_file_fptr (FILE *f, long *length, bool isstring, char * buffer,
       long offset) {
  if (f) {                         // is it a good file?
    long filesize = get_length_fptr(f);   // get the size
    if (*length == 0 || *length > filesize)
      *length = filesize;
    if (!buffer) {                // if memory not provided
       long size = *length +
           (isstring?1:0);        // for char data, add a null terminator
       buffer = malloc(size);     // allocate memory for a buffer
    }
    if (fseek(f, offset, SEEK_SET) != offset)
      fprintf(stderr, "OFFSET NOT AS ASKED\n");;
    // read file into buffer from the offset on: not 1 item == bad read:
    if (fread(buffer, *length, 1, f) != 1) {
        // not exactly 1 `object' of size (*length)
        perror ("failed to read file");
        free (buffer);
        *length = 0;
        buffer = NULL;
    }
    // fprintf(stderr, "length read = %ld\n", *length);
    if (isstring)
        buffer [*length] = '\0';       // complete the string if required
    return buffer;
  } else {                     // No. Bad file, no biscuit.
      perror ("failed to open file");
      return NULL;
  }
}
