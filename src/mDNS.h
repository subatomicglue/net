
#include <map>
#include <vector>
#include <cstdint>
#include "platform_check.h"
#include "utils.h"
#include "mDNSData.h"

class mDNS {
public:
  mDNS() {
    // setup defaults (can be overridden)
    rawCallbacks.push_back( printf_cb );
    questionCallbacks.push_back( printf_qCb );
    recordCallbacks.push_back( printf_rCb );
  }
  int recv();
  int send( const char* msg, size_t msg_size );

  // Raw mDNS message callbacks
  // called for each message.   May contain multiple records, use qCb or rCb to access them individually.
  std::vector<DNSHeader::Callback> rawCallbacks;

  // question callback
  // called for each question recv'd via mDNS
  std::vector<DNSQuestion::Callback> questionCallbacks;

  // records callback (e.g. for record types of answer, authority, additional)
  // called for each record recv'd via mDNS
  std::vector<DNSResourceRecord::Callback> recordCallbacks;

  // built in Raw mDNS callback - for printf debugging or logging
  DNSHeader::Callback printf_cb = []( const std::string& sender_ip, const char* buffer, uint16_t buffer_size ) {
    printf( "Received mDNS message: \n" );
    cppArrayDump( buffer, buffer_size );
    printf( "\n" );
  };

  // built in question callback - for printf debugging or logging
  DNSQuestion::Callback printf_qCb = []( const std::string& sender_ip, const std::string& name, uint16_t qtype, uint16_t qclass, bool flushbit, const char* buffer, uint16_t buffer_size, int pos ) {
    printf( "%s:\n", DNSHeader::typeLookup( DNSHeader::Type::QUESTION ).c_str() );
    printf( "  Name: %s\n", name.c_str() );
    printf( "  Type:  0x%04x, %d, %s\n", (uint16_t)qtype, (uint16_t)qtype, DNSQuestion::typeLookup( qtype ).c_str() );
    printf( "  Class: 0x%04x, %d, %s%s\n", (uint16_t)qclass, (uint16_t)qclass, DNSQuestion::classLookup( qclass ).c_str(), flushbit ? " +FLUSHBIT" : "" );
  };

  // built in records callback (e.g. for record types of answer, authority, additional)  - for printf debugging or logging
  DNSResourceRecord::Callback printf_rCb = []( const std::string& sender_ip, DNSHeader::Type msg_type, const std::string& name, uint16_t rtype, uint16_t rclass, bool flushbit, uint32_t ttl, std::vector<uint8_t>& data, const char* buffer, uint16_t buffer_size, int pos ) {
    printf( "%s:\n", DNSHeader::typeLookup( msg_type ).c_str() );
    printf( "  Name: %s\n", name.c_str() );
    printf( "  Type:  0x%04x, %d, %s\n", (uint16_t)rtype, (uint16_t)rtype, DNSQuestion::typeLookup( rtype ).c_str() );
    printf( "  Class: 0x%04x, %d, %s%s\n", (uint16_t)rclass, (uint16_t)rclass, DNSQuestion::classLookup( rclass ).c_str(), flushbit ? " +FLUSHBIT" : "" );
    printf( "  TTL: %d\n", (uint32_t)ttl );
    printf( "  Data length: %d\n", (uint16_t)data.size() );

    uint16_t rdlength = data.size();
    switch (rtype) {
      case DNSQuestion::Type::A:
        printf( "  Address: %s\n", ipv4_NetToStr( &buffer[pos] ).c_str() );
        break;
      case DNSQuestion::Type::TXT:
        printf( "  TXT Data: %s\n", std::string( buffer + pos, rdlength ).c_str() );
        if (rdlength < 100)
          hexDump( &buffer[pos], rdlength );
        else
          printf( "    - %d bytes (not showing, too long)\n", rdlength );
        break;
      case DNSQuestion::Type::PTR: {
        std::string ptrname = parseDomainName(buffer, pos, buffer_size);
        printf( "  PTR Name: %s\n", ptrname.c_str() );
        break;
      }
      case DNSQuestion::Type::AAAA:
        printf( "  Address: %s\n", ipv6_NetToStr( &buffer[pos] ).c_str() );
        break;
      case DNSQuestion::Type::SRV: {
        uint16_t priority = ntohs(*(uint16_t*)&buffer[pos]);
        pos += 2;
        uint16_t weight = ntohs(*(uint16_t*)&buffer[pos]);
        pos += 2;
        uint16_t port = ntohs(*(uint16_t*)&buffer[pos]);
        pos += 2;
        //int temp_pos = pos + 6;
        std::string target = parseDomainName(buffer, pos, buffer_size);
        printf( "  Priority: %u\n", priority );
        printf( "  Weight: %u\n", weight );
        printf( "  Port: %u\n", port );
        printf( "  Target: %s\n", target.c_str() );
        break;
      }
      case DNSQuestion::Type::OPT: {
        // OPT record is typically used for EDNS0 options and may contain multiple options
        // For simplicity, we print the raw data
        printf( "  OPT Data: " );
        if (rdlength < 100)
          hexDump( &buffer[pos], rdlength );
        else
          printf( "    - %d bytes (not showing, too long)\n", rdlength );
        break;
      }
      case DNSQuestion::Type::NSEC: {
        //int temp_pos = pos;
        std::string nextDomainName = parseDomainName(buffer, pos, buffer_size);
        printf( "  Next Domain Name: %s\n", nextDomainName.c_str() );
        printf( "  Type Bitmaps:\n" );
        if (rdlength < 100)
          hexDump( &buffer[pos], rdlength );
        else
          printf( "    - %d bytes (not showing, too long)\n", rdlength );
        break;
      }
      case DNSQuestion::Type::ANY: {
        // ANY is a request for all records, so data parsing depends on response
        printf( "  ANY Data: " );
        if (rdlength < 100)
          hexDump( &buffer[pos], rdlength );
        else
          printf( "    - %d bytes (not showing, too long)\n", rdlength );
        break;
      }
      default:
        printf( "  Raw data: \n" );
        if (rdlength < 100)
          hexDump( &buffer[pos], rdlength );
        else
          printf( "    - %d bytes (not showing, too long)\n", rdlength );
    }
  };

private:
  DNSHeader::Callback mCb = [this]( const std::string& sender_ip, const char* buffer, uint16_t buffer_size ) {
    for (auto func : rawCallbacks) {
      func( sender_ip, buffer, buffer_size );
    }
  };
  DNSQuestion::Callback mqCb = [this]( const std::string& sender_ip, const std::string& name, uint16_t qtype, uint16_t qclass, bool flushbit, const char* buffer, uint16_t buffer_size, int pos ) {
    for (auto func : questionCallbacks) {
      func( sender_ip, name, qtype, qclass, flushbit, buffer, buffer_size, pos );
    }
  };
  DNSResourceRecord::Callback mrCb = [this]( const std::string& sender_ip, DNSHeader::Type msg_type, const std::string& name, uint16_t rtype, uint16_t rclass, bool flushbit, uint32_t ttl, std::vector<uint8_t>& data, const char* buffer, uint16_t buffer_size, int pos ) {
    for (auto func : recordCallbacks) {
      func( sender_ip, msg_type, name, rtype, rclass, flushbit, ttl, data, buffer, buffer_size, pos );
    }
  };
};

///////////////////////////////////////////////////////////////////////////////////////

#if HAS_ASIO==1
#include <iostream>
#include <asio.hpp>

#include <iostream>
#include <asio.hpp>

int mDNS::send( const char* msg, size_t msg_size ) {
  try {
    asio::io_context io_context;

    asio::ip::udp::socket socket(io_context);
    socket.open(asio::ip::udp::v4());

    asio::ip::address multicast_address = asio::ip::make_address("224.0.0.251");
    asio::ip::udp::endpoint endpoint(multicast_address, 5353);

    socket.send_to(asio::buffer(msg, msg_size), endpoint);

    std::cout << "Message sent successfully." << std::endl;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}

int mDNS::recv() {
  try {
      asio::io_context io_context;

      asio::ip::udp::socket socket(io_context);
      socket.open(asio::ip::udp::v4());

      asio::ip::udp::endpoint listen_endpoint(asio::ip::udp::v4(), 5353); // Example multicast port
      socket.set_option(asio::ip::udp::socket::reuse_address(true));
      socket.bind(listen_endpoint);

      asio::ip::address multicast_address = asio::ip::make_address("224.0.0.251"); // Example multicast address
      socket.set_option(asio::ip::multicast::join_group(multicast_address));

      char buffer[1024];
      asio::ip::udp::endpoint sender_endpoint;

      while (true) {
        int bytesReceived = socket.receive_from(asio::buffer(buffer, sizeof( buffer )), sender_endpoint);
        if (bytesReceived < 0) {
          fprintf(stderr, "receive_from failed.  bytesreceived: %d  Error code: %s\n", bytesReceived, strerror(errno) );
          break;
        }

        int it = 0;
        parseMDNSPacket( buffer, it, bytesReceived, ip_NetToStr( (sockaddr&)senderAddr ), mCb, mqCb, mrCb );
      }
    } catch (std::exception& e) {
      std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

#elif IS_POSIX==1
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

int mDNS::send( const char* msg, size_t msg_size ) {
  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    fprintf( stderr, "Socket creation failed. Error code: %d\n", errno );
    return 1;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5353);
  ::inet_pton(AF_INET, "224.0.0.251", &addr.sin_addr);

  if (::sendto(sock, msg, msg_size, 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
    fprintf( stderr, "sendto failed.  Error code: %s\n", strerror(errno) );
    return 1;
  }

  ::close(sock);
  return 0;
}

int mDNS::recv() {
  int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    fprintf( stderr, "Socket creation failed.  Error code: %s\n", strerror(errno));
    return 1;
  }

  // reuse port/address (when binding)
  // NOTE:   on MacOS, PORT case has to be first or it fails to bind
  int opt = 1;
#if defined( SO_REUSEPORT )
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&opt, sizeof(opt)) < 0) {
    fprintf( stderr, "setsockopt(SO_REUSEPORT) failed.  Error code: %s\n", strerror(errno));
    return 1;
  }
#elif defined( SO_REUSEADDR )
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
    fprintf( stderr, "setsockopt(SO_REUSEADDR) failed.  Error code: %s\n", strerror(errno));
    return 1;
  }
#endif
	unsigned char ttl = 1;
	unsigned char loopback = 1;
	setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));
	setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loopback, sizeof(loopback));

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5353); // mDNS port
  addr.sin_addr.s_addr = INADDR_ANY;
#ifdef __APPLE__
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

  ip_mreq mreq;
  inet_pton(AF_INET, "224.0.0.251", &mreq.imr_multiaddr); // Example multicast address
  mreq.imr_interface.s_addr = INADDR_ANY;
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    fprintf( stderr, "setsockopt failed.  Error code: %s\n", strerror(errno));
    ::close(sock);
    return 1;
  }
  setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&addr.sin_addr, sizeof(addr.sin_addr));

  if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "Bind failed.  Error code: %s\n", strerror(errno));
    ::close(sock);
    return 1;
  }

// switch to ASYNC (non blocking) mode
// #ifdef _WIN32
// 	unsigned long param = 1;
// 	ioctlsocket(sock, FIONBIO, &param);
// #else
// 	::fcntl( sock, F_SETFL, ::fcntl(sock, F_GETFL, 0) | O_NONBLOCK );
// #endif


  char buffer[1024];
  sockaddr_in senderAddr;
  socklen_t senderAddrSize = sizeof(senderAddr);

  while (true) {
    int bytesReceived = ::recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
    if (bytesReceived < 0) {
      fprintf(stderr, "recvfrom failed.  bytesreceived: %d  Error code: %s\n", bytesReceived, strerror(errno) );
      break;
    }

    int it = 0;
    parseMDNSPacket( buffer, it, bytesReceived, ip_NetToStr( (sockaddr&)senderAddr ), mCb, mqCb, mrCb );
  }

  ::close(sock);
  return 0;
}

#elif IS_WINDOWS==1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int mDNS::send( const char* msg, size_t msg_size ) {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf( stderr, "WSAStartup failed." );
    return 1;
  }

  SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == INVALID_SOCKET) {
    fprintf( stderr, "Socket creation failed.\n" );
    WSACleanup();
    return 1;
  }

  sockaddr_in multicastAddr;
  multicastAddr.sin_family = AF_INET;
  multicastAddr.sin_port = htons(5353);
  inet_pton(AF_INET, "224.0.0.251", &multicastAddr.sin_addr);

  if (sendto(sock, msg, msg_size, 0, (sockaddr*)&multicastAddr, sizeof(multicastAddr)) == SOCKET_ERROR) {
    fprintf( stderr, "sendto failed.\n" );
  }

  closesocket(sock);
  WSACleanup();
  return 0;
}

int mDNS::recv() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf( stderr, "WSAStartup failed.\n" );
    return 1;
  }

  SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == INVALID_SOCKET) {
    fprintf( stderr, "Socket creation failed.\n" );
    WSACleanup();
    return 1;
  }

  sockaddr_in recvAddr;
  recvAddr.sin_family = AF_INET;
  recvAddr.sin_port = htons(5353); // Example multicast port
  recvAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
    fprintf( stderr, "Bind failed.\n" );
    closesocket(sock);
    WSACleanup();
    return 1;
  }

  ip_mreq mreq;
  inet_pton(AF_INET, "224.0.0.251", &mreq.imr_multiaddr); // Example multicast address
  mreq.imr_interface.s_addr = INADDR_ANY;

  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
    fprintf( stderr, "setsockopt failed.\n" );
    closesocket(sock);
    WSACleanup();
    return 1;
  }

  char buffer[1024];
  sockaddr_in senderAddr;
  int senderAddrSize = sizeof(senderAddr);

  while (true) {
    int bytesReceived = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
    if (bytesReceived == SOCKET_ERROR) {
      fprintf(stderr, "recvfrom failed.  bytesreceived: %d\n", bytesReceived );
      break;
    }

    int it = 0;
    parseMDNSPacket( buffer, it, bytesReceived, ip_NetToStr( (sockaddr&)senderAddr ), mCb, mqCb, mrCb );
  }

  closesocket(sock);
  WSACleanup();
  return 0;
}
#endif
