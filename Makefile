##############################################################
# native sockets build (posix or winsock)...
all: tcp udp mdns

udp: src/main_udp.cpp src/UDP.h src/utils.h src/platform_check.h
	g++ -g src/main_udp.cpp -pthread -std=c++17 -oudp

tcp: src/main_tcp.cpp src/TCP.h src/utils.h src/platform_check.h
	g++ -g src/main_tcp.cpp -pthread -std=c++17 -otcp

mdns: src/main_mdns.cpp src/mDNS.h src/mDNSData.h src/utils.h src/platform_check.h src/mDNSTestData.h
	g++ -g src/main_mdns.cpp -pthread -std=c++17 -omdns

##############################################################
# asio build
# https://docs.conan.io/2.0/reference/commands/install.html
# https://docs.conan.io/2.0/reference/commands/build.html
conan:
	rm -rf ./build
	mkdir -p build
	cd build && conan install .. --build=missing --settings=build_type=Debug; cd  -
	cd build && conan build .. ; cd -
	cd build && source ./Debug/generators/conanbuild.sh; cd -
	cd build/Debug && cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug; cd -
	cd build/Debug && cmake --build .; cd -

clean:
	rm -rf ./build
	rm -f ./udp ./tcp ./mdns
	rm -rf ./mdns.dSYM ./tcp.dSYM ./udp.dSYM

