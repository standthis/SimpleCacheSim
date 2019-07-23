/* get_args.h
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

#ifndef get_args_h
#define get_args_h

#include <stdio.h>

// get the command line and get ready to initialize values needed to get started
// returns the configuration read in as an array of strings, one per line
char** get_args (int argc, char *argv[]);

#endif // get_args_h
