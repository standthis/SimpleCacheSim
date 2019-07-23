/* IOutils.c
 * 
 * Open a file for reading or writing and find out its length.
 *
 * Philip Machanick
 * June 2018
 *
 */

#include "IOutils.h"
#include "stringutils.h"

#include <stdbool.h>
#include <sys/stat.h> // for stat
#include <stdio.h>

// idea from
// http://stackoverflow.com/questions/9840629/create-a-file-if-one-doesnt-exist-c

// open a file to write only if it does not already exist
// POTENTIAL RACE CONDITION: if another process opens it
// between the check and opening to write
FILE *openWrite (char name[], char * extension) {
    FILE *fptr;
    char *fullname = mystrcat (name, extension);
    fptr = fopen(fullname, "rb+");
    if (!fptr) {  // if file does not exist, create it
        fptr = fopen(fullname, "wb");
        if (!fptr) {
           fprintf(stderr, "error opening file `%s' ", fullname);
           perror ("");
           return NULL;
        }
    } else {
        fprintf(stderr, "error opening file `%s' exists already\n", fullname);
        fclose (fptr);
        return NULL;
    }
    return fptr;
}

// open a file to read; return NULL on failure
FILE *openRead (char name[], char * extension) {
    FILE *fptr;
    char *fullname = mystrcat (name, extension);
    fptr = fopen(fullname, "rb+");
    if (!fptr) {  // if NULL pointer, error
        fprintf(stderr, "error opening file `%s' ", fullname);
        perror ("");
        return NULL;
    }
    return fptr;
}


// find out how long a file is without opening it
// basic idea from
// http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
long get_length (char name[]) {
  struct stat fileinfo;
  if (!stat (name, &fileinfo)) {       // is it a good file?
    return fileinfo.st_size;           // yes: return size in bytes
  } else {
    perror ("can't get size of file");
    return 0;
  }
}

// same but if we have a file pointer not a name
long get_length_fptr (FILE *f) {
  struct stat fileinfo;
  if (!fstat (fileno(f), &fileinfo)) {       // is it a good file?
    return fileinfo.st_size;           // yes: return size in bytes
  } else {
    perror ("can't get size of file (ptr)");
    return 0;
  }
}
