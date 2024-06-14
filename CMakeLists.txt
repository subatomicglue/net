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

# APPLE DEVELOPER code signing - stop annoying popups
# https://github.com/tony-go/codesign-macos
set(TEAM_ID "XXXX" CACHE STRING "The development team ID for code signing")
if(TEAM_ID STREQUAL "")
    message(FATAL_ERROR "Please set the TEAM_ID variable when generating the project")
endif()
set(CMAKE_MACOSX_BUNDLE NO)
set(XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
set(XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${TEAM_ID})
set(XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
set(XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED TRUE)


set(APP_NAME "mdns")
add_executable(${APP_NAME} src/main_mdns.cpp)
target_link_libraries(${APP_NAME} ${ASIO_LIBS})
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)
set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE ${XCODE_ATTRIBUTE_CODE_SIGN_STYLE}
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY}
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED ${XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED}
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--timestamp=http://timestamp.apple.com/ts01  --options=runtime,library"
    XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS "NO"
)

set(APP_NAME "tcp")
add_executable(${APP_NAME} src/main_tcp.cpp)
target_link_libraries(${APP_NAME} ${ASIO_LIBS})
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)

set(APP_NAME "udp")
add_executable(${APP_NAME} src/main_udp.cpp)
target_link_libraries(${APP_NAME} ${ASIO_LIBS})
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)
