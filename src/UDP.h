
#include "platform_check.h"

class UDP {
public:
    int recv();
    int send( const char* msg, size_t msg_size );
};

#if HAS_ASIO==1
#include <iostream>
#include <asio.hpp>

#include <iostream>
#include <asio.hpp>

int UDP::send( const char* msg, size_t msg_size ) {
    try {
        asio::io_context io_context;

        asio::ip::udp::socket socket(io_context);
        socket.open(asio::ip::udp::v4());

        asio::ip::udp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 12345);

        socket.send_to(asio::buffer(msg, msg_size), endpoint);

        std::cout << "Message sent successfully." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

int UDP::recv() {
    try {
        asio::io_context io_context;

        asio::ip::udp::socket socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 5353));

        char buffer[512];
        asio::ip::udp::endpoint senderEndpoint;

        while (true) {
            size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);
            buffer[len] = '\0'; // Null-terminate the received data
            std::cout << "Received message: " << buffer << std::endl;
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
#include <errno.h>

int UDP::send( const char* msg, size_t msg_size ) {
  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    std::cerr << "Socket creation failed." << std::endl;
    printf("Error code: %d\n", errno);
    return 1;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  ::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  if (::sendto(sock, msg, msg_size, 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
    std::cerr << "sendto failed." << std::endl;
    printf("Error code: %d\n", errno);
  } else {
    std::cout << "Message sent successfully." << std::endl;
  }

  ::close(sock);
  return 0;
}

int UDP::recv() {
  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    std::cerr << "Socket creation failed." << std::endl;
    printf("Error code: %d\n", errno);
    return 1;
  }

  // reuse
  int opt = 1;
#if defined( SO_REUSEPORT )
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&opt, sizeof(opt)) < 0) {
    printf("setsockopt(SO_REUSEPORT) failed.  Error code: %d\n", errno);
    return 1;
  }
#elif defined( SO_REUSEADDR )
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
    printf("setsockopt(SO_REUSEADDR) failed.  Error code: %d\n", errno);
    return 1;
  }
#endif

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  //addr.sin_addr.s_addr = INADDR_ANY;
  ::inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);

  if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
    std::cerr << "Bind failed." << std::endl;
    printf("Error code: %d\n", errno);
    ::close(sock);
    return 1;
  }

  char buffer[1024];
  sockaddr_in senderAddr;
  socklen_t senderAddrSize = sizeof(senderAddr);

  while (true) {
    printf( "Receiving...\n" );
    int bytesReceived = ::recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&senderAddr, &senderAddrSize);
    if (bytesReceived < 0) {
      std::cerr << "recvfrom failed." << std::endl;
      printf("Error code: %d\n", errno);
      break;
    }
    buffer[bytesReceived] = '\0';
    std::cout << "Received: " << buffer << std::endl;
  }

  ::close(sock);
  return 0;
}

#elif IS_WINDOWS==1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int UDP::send( const char* msg, size_t msg_size ) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (sendto(sock, msg, msg_size, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "sendto failed." << std::endl;
    } else {
        std::cout << "Message sent successfully." << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

int UDP::recv() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(5353); // mDNS port
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[512];
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    while (true) {
        int recvLen = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
        if (recvLen == SOCKET_ERROR) {
            std::cerr << "recvfrom failed." << std::endl;
            break;
        }

        buffer[recvLen] = '\0'; // Null-terminate the received data
        std::cout << "Received message: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
#endif
