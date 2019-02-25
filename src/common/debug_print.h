
#ifndef DEBUG_PRINT_HHH
#define DEBUG_PRINT_HHH

#ifdef DEBUG

#define DEBUG_PRINT(...)\
    do{\
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    }while(0)

#define debug_print(...) do{ fprintf(stderr, __VA_ARGS__); }while(0)

#else

#define DEBUG_PRINT(...) do{ }while(0)

#define debug_print(...) do{ }while(0)

#endif


#endif

