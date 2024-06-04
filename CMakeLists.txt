cmake_minimum_required( VERSION 3.15 )
project( net CXX )
#find_package( ZLIB REQUIRED )
find_package( asio REQUIRED )

add_executable(mdns main_mdns.cpp)
target_link_libraries(mdns asio::asio)
target_compile_features(mdns PRIVATE cxx_std_17)

add_executable(tcp main_tcp.cpp)
target_link_libraries(tcp asio::asio)
target_compile_features(tcp PRIVATE cxx_std_17)

add_executable(udp main_udp.cpp)
target_link_libraries(udp asio::asio)
target_compile_features(udp PRIVATE cxx_std_17)
