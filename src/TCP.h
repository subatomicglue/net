
#include "platform_check.h"

class TCP {
public:
    int recv();
    int send( const char* msg, size_t msg_size );
};

#if HAS_ASIO==1
#include <iostream>
#include <asio.hpp>

#include <iostream>
#include <asio.hpp>

int TCP::send( const char* msg, size_t msg_size ) {
    try {
        asio::io_context io_context;

        asio::ip::tcp::socket socket(io_context);
        asio::ip::tcp::resolver resolver(io_context);
        asio::connect(socket, resolver.resolve("127.0.0.1", "12345"));

        asio::write(socket, asio::buffer(msg, msg_size));

        std::cout << "Message sent successfully." << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

int TCP::recv() {
  try {
    asio::io_context io_context;

    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 12345));

    while (true) {
      asio::ip::tcp::socket socket(io_context);
      acceptor.accept(socket);

      char buffer[512];
      size_t recvLen;
      while ((recvLen = socket.read_some(asio::buffer(buffer))) > 0) {
          buffer[recvLen] = '\0'; // Null-terminate the received data
          std::cout << "Received message: " << buffer << std::endl;
      }
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

int TCP::send( const char* msg, size_t msg_size ) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    ::inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (::connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        close(sock);
        return 1;
    }

    if (::send(sock, msg, msg_size, 0) < 0) {
        std::cerr << "send failed." << std::endl;
    } else {
        std::cout << "Message sent successfully." << std::endl;
    }

    ::close(sock);
    return 0;
}

int TCP::recv() {
  int serverSock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (serverSock < 0) {
    std::cerr << "Socket creation failed." << std::endl;
    return 1;
  }

  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(12345); // Example port
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (::bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Bind failed." << std::endl;
    ::close(serverSock);
    return 1;
  }

  if (::listen(serverSock, SOMAXCONN) < 0) {
    std::cerr << "Listen failed." << std::endl;
    ::close(serverSock);
    return 1;
  }

  int clientSock;
  sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);

  while (true) {
    clientSock = ::accept(serverSock, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSock < 0) {
      std::cerr << "Accept failed." << std::endl;
      ::close(serverSock);
      return 1;
    }

    char buffer[512];
    ssize_t recvLen;
    while ((recvLen = ::recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
      buffer[recvLen] = '\0'; // Null-terminate the received data
      std::cout << "Received message: " << buffer << std::endl;
    }

    if (recvLen < 0) {
      std::cerr << "Recv failed." << std::endl;
      ::close(clientSock);
      ::close(serverSock);
      return 1;
    }

    ::close(clientSock);
  }

  ::close(serverSock);
  return 0;
}

#elif IS_WINDOWS==1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int TCP::send( const char* msg, size_t msg_size ) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (send(sock, msg, msg_size, 0) == SOCKET_ERROR) {
        std::cerr << "send failed." << std::endl;
    } else {
        std::cout << "Message sent successfully." << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

int TCP::recv() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed." << std::endl;
    return 1;
  }

  SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSock == INVALID_SOCKET) {
    std::cerr << "Socket creation failed." << std::endl;
    WSACleanup();
    return 1;
  }

  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(12345); // Example port
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "Bind failed." << std::endl;
    closesocket(serverSock);
    WSACleanup();
    return 1;
  }

  if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR) {
    std::cerr << "Listen failed." << std::endl;
    closesocket(serverSock);
    WSACleanup();
    return 1;
  }

  SOCKET clientSock;
  sockaddr_in clientAddr;
  int clientAddrSize = sizeof(clientAddr);

  while (true) {
    clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSock == INVALID_SOCKET) {
      std::cerr << "Accept failed." << std::endl;
      closesocket(serverSock);
      WSACleanup();
      return 1;
    }

    char buffer[512];
    int recvLen;
    while ((recvLen = recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
      buffer[recvLen] = '\0'; // Null-terminate the received data
      std::cout << "Received message: " << buffer << std::endl;
    }

    if (recvLen == SOCKET_ERROR) {
      std::cerr << "Recv failed." << std::endl;
      closesocket(clientSock);
      closesocket(serverSock);
      WSACleanup();
      return 1;
    }

    closesocket(clientSock);
  }

  closesocket(serverSock);
  WSACleanup();
  return 0;
}
#endif
