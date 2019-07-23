/* error.h
 *
 * Basic error processing – print a message containing a passed in
 * string to augment the message based on the error code; optionally
 * terminate the program. Error codes defined here for use in other code.
 * Use false for the noexit parameter to fill the program. To report
 * where the error was triggered, used __LINE__ and __FILE__ macros.
 * All strings can be passed as NULL and a 0 line number skips reporting
 * the line number.
 * 
 * Philip Machanick
 * June 2018
 *
 */

#ifndef error_h
#define error_h

#include <stdbool.h>

enum ErrorCodes {badblockcount, badcachesize, badblockindex, badCacheID, badAssociativity,
                 associativityError, configError, configFileError, workloadError,
                 statsLevelError};

// if line number is 0, skip printing it; if filename or text is NULL skip them too
// relies on errorcode aligning with an error string in the C file; reports if
// the value is out of range
void error (unsigned errorcode, bool noexit, char* text,
            unsigned linenumber, char* filename);

#endif // error_h