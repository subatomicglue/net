##############################################################
# native sockets build (posix or winsock)...
all:
	rm -rf ./build
	mkdir -p build
	conan install . --output-folder build/Debug/generators --build=missing --settings=build_type=Debug
	conan build . --output-folder build/Debug/generators
	cd build && source ./Debug/generators/conanbuild.sh; cd -
	cd build/Debug && cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug; cd -
	cd build/Debug && cmake --build .; cd -

##############################################################
# asio build
# https://docs.conan.io/2.0/reference/commands/install.html
# https://docs.conan.io/2.0/reference/commands/build.html
asio:
	rm -rf ./build-asio
	mkdir -p build-asio
	conan install . --output-folder build-asio/Debug/generators --build=missing --settings=build_type=Debug --options asio=True
	conan build . --output-folder build-asio/Debug/generators
	cd build-asio && source ./Debug/generators/conanbuild.sh; cd -
	cd build-asio/Debug && cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug; cd -
	cd build-asio/Debug && cmake --build .; cd -

clean:
	rm -rf ./build ./build-asio
	rm -f ./udp ./tcp ./mdns
	rm -rf ./mdns.dSYM ./tcp.dSYM ./udp.dSYM

