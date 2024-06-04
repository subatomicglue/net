cmake_minimum_required( VERSION 3.15 )
project( net CXX )
#find_package( ZLIB REQUIRED )

# conanfile.py's asio is optional, so it may not be dropped into the path
if(EXISTS ${CMAKE_MODULE_PATH}/asio-config.cmake)
  find_package( asio ) # Without 'REQUIRED'! we can built with or without it
  IF( asio_FOUND )
  SET( ASIO_LIBS asio::asio )
  ENDIF()
endif()

add_executable(mdns src/main_mdns.cpp)
target_link_libraries(mdns ${ASIO_LIBS})
target_compile_features(mdns PRIVATE cxx_std_17)

add_executable(tcp src/main_tcp.cpp)
target_link_libraries(tcp ${ASIO_LIBS})
target_compile_features(tcp PRIVATE cxx_std_17)

add_executable(udp src/main_udp.cpp)
target_link_libraries(udp ${ASIO_LIBS})
target_compile_features(udp PRIVATE cxx_std_17)
