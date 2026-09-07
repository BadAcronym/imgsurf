#ifndef IMGSURF_PRINT_MACROS
#define IMGSURF_PRINT_MACROS

// taken from the datasurf implementation
#define __STRINGIFY(x) #x
#define __TO_STRING(x) __STRINGIFY(x)
#define __LINE_STR __TO_STRING(__LINE__)
#define __LOCATION__ "[" __FILE__ ":" __LINE_STR "]"

#define FIRST(...) FIRST_HELPER(__VA_ARGS__, throwaway)
#define FIRST_HELPER(first, ...) first

#define REST(...) REST_HELPER(NUM(__VA_ARGS__), __VA_ARGS__)
#define REST_HELPER(qty, ...) REST_HELPER2(qty, __VA_ARGS__)
#define REST_HELPER2(qty, ...) REST_HELPER_##qty(__VA_ARGS__)
#define REST_HELPER_ONE(first)
#define REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define NUM(...) \
    SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#define IM_ERROR(...) \
        fprintf(stderr, "\n\033[31;1m" __LOCATION__ "\n\033[31;1;7mERROR: " \
                FIRST(__VA_ARGS__) "\033[0m\n" REST(__VA_ARGS__))

#define IM_WARN(...) \
        fprintf(stdout, "\033[33;1m" __LOCATION__ "\n\033[33;1;7mWARNING: " \
                FIRST(__VA_ARGS__) "\033[0m\n" REST(__VA_ARGS__))

#ifdef DEBUG
    #define IM_DEBUG(...) \
            printf(FIRST(__VA_ARGS__) "\n" REST(__VA_ARGS__))
#else
    #define IM_DEBUG(...)
#endif

#endif
