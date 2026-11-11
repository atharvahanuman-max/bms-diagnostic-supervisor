#ifndef STATUS_H
#define STATUS_H

/* Common status codes returned by functions in every module. */
typedef enum
{
    STATUS_SUCCESS = 0,        /* operation completed */
    STATUS_NULL_POINTER,       /* a required pointer argument was NULL */
    STATUS_INVALID_ARGUMENT,   /* argument value not acceptable */
    STATUS_PARSE_ERROR,        /* text could not be converted to data */
    STATUS_OUT_OF_RANGE,       /* value outside its plausible range */
    STATUS_FILE_ERROR,         /* file could not be opened, read or written */
    STATUS_CAPACITY_FULL,      /* fixed-size storage is full */
    STATUS_NO_DATA             /* required data has not been loaded yet */
} Status_t;

#endif
