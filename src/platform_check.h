#include <cstdio>

// Check for Windows
#if defined(_WIN32) || defined(_WIN64)
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#endif

// Check for POSIX (Linux or macOS)
#if defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#include <unistd.h>
#if defined(_POSIX_VERSION)
#define IS_POSIX 1
#else
#define IS_POSIX 0
#endif
#else
#define IS_POSIX 0
#endif

// Check for standalone ASIO (without Boost)
#if defined(ASIO_STANDALONE)
#define HAS_ASIO 1
#else
#define HAS_ASIO 0
#endif
