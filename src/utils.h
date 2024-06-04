#ifndef SUBA_NET_UTILS
#define SUBA_NET_UTILS

#include <string>

inline bool isLittleEndian()
{
    const int value = 0x01;
    const void * address = static_cast<const void *>(&value);
    const unsigned char * least_significant_address = static_cast<const unsigned char *>(address);
    return (*least_significant_address == 0x01);
}

inline bool isBigEndian() {
  return !isLittleEndian();
}

template <typename T>
inline T swapEndian( T val ) {
  if (isLittleEndian())
    for (int x = 0; x < sizeof( T ); x += 2) {
      char temp = ((char*)&val)[x];
      ((char*)&val)[x] = ((char*)&val)[x+1];
      ((char*)&val)[x+1] = temp;
    }
  return val;
}
//for (int x = sizeof(DNSHeader); x < bytesReceived; ++x)
void hexDump(const char* data, size_t len) {
  const size_t width = 16;
  for (size_t j = 0; j < len; j += width) {
    for (size_t x = j; x < (j + width); ++x)
      printf( "0x%02x ", (uint8_t)data[x] );
    printf( "| " );
    for (size_t x = j; x < (j + width); ++x)
      printf( "%c", (char)(32 <= data[x] && data[x] <= 126 ? data[x] : '.') );
    printf( " |\n" );
  }
}

void cppArrayDump(const char* data, size_t len) {
  printf( "{ " );
  const size_t width = 16;
  for (size_t j = 0; j < len; j += width) {
    for (size_t x = j; x < (j + width); ++x)
      printf( "0x%02x, ", (uint8_t)data[x] );
  }
  printf( "}" );
}

#if IS_POSIX==1
#include <arpa/inet.h>
inline std::string ipv6_NetToStr( const char* buffer ) {
  char addr_str[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, buffer, addr_str, sizeof(addr_str));
  return std::string( addr_str );
}
inline std::string ipv4_NetToStr( const char* buffer ) {
  char buf[16];
  snprintf( buf, sizeof( buf ), "%d.%d.%d.%d", (uint8_t)buffer[0], (uint8_t)buffer[1], (uint8_t)buffer[2], (uint8_t)buffer[3] );
  return std::string( buf );
}
#elif IS_WINDOWS==1
#include <arpa/inet.h>
inline std::string ipv6_NetToStr( const char* buffer ) {
  char addr_str[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, buffer, addr_str, sizeof(addr_str));
  return std::string( addr_str );
}
inline std::string ipv4_NetToStr( const char* buffer ) {
  char buf[16];
  snprintf( buf, sizeof( buf ), "%d.%d.%d.%d", (uint8_t)buffer[0], (uint8_t)buffer[1], (uint8_t)buffer[2], (uint8_t)buffer[3] );
  return std::string( buf );
}
#endif

#endif
