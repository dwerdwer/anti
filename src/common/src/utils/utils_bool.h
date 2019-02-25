#ifndef UTILS_BOOL_HHH
#define UTILS_BOOL_HHH

// Note: if system header <stdbool.h> exists, 
// use 'bool', 'true/false' instead of this 'boolean_t', 'YES/NO'.
//
// Since return value of C interfaces usually is not boolean,
// it does not recommand to use 'YES/NO' or 'true/false' as the return value of interfaces for utils_xxx and
// any other C like interfaces. 
// Here, set its value to not contain 0 to avoid it is used 
// as the return value of any C interfaces.
typedef enum
{
    NO = 2,
    YES = 3,
}boolean_t;

#endif

