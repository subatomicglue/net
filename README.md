# transport lib
Currently just a sandbox, playing around with various transport protocols.


Explorations of:
- TCP
- UDP
- mDNS (Service Discovery)

Implementations:
- Posix (Linux and MacOS)
- asio  (non boost)
- Winsock (sorry...  written, but untested)

Developed on MacOS, YMMV for other platforms (you may need to fix what I haven't).


# How To Build
Build using posix sockets for Linux/MacOS
```
make
cd build

# try one of the demos
./udp
```

Build using ASIO (non boost), using conan package manager & cmake
```
make asio
cd build-asio/Debug

# try one of the demos
./udp
```

Build using winsock for Windows
```
TODO   (sorry)
```


# mDNS-SD spec
Multicast DNS
https://www.rfc-editor.org/rfc/rfc6762.txt

DNS-Based Service Discovery
https://www.rfc-editor.org/rfc/rfc6763.txt


# notes
```
sudo nmap -Pn -sUC -p5353  localhost

dns-sd -B _services._dns-sd._udp


_services._dns-sd._udp.local
```


# mDNS:  Example Messages
There is this nice mDNS demo: [https://github.com/mjansson/mdns](https://github.com/mjansson/mdns)
```
./mdns --dump
```

Which can monitor local network mDNS traffic
```
192.168.4.115:56887: Question A mantis.local. rclass 0x1 ttl 0

192.168.4.115:58749: Question PTR _services._dns-sd._udp.local. rclass 0x1 ttl 0
192.168.4.30:5353: Answer PTR _services._dns-sd._udp.local. rclass 0x1 ttl 300
192.168.4.30:5353: Answer PTR _myq._tcp.local. rclass 0x1 ttl 300
192.168.4.30:5353: Answer TXT MyQ-35E._myq._tcp.local. rclass 0x8001 ttl 300
192.168.4.30:5353: Answer <UNKNOWN> MyQ-35E._myq._tcp.local. rclass 0x8001 ttl 300
192.168.4.30:5353: Answer SRV MyQ-35E._myq._tcp.local. rclass 0x8001 ttl 300
192.168.4.30:5353: Additional A MyQ-35E.local. rclass 0x8001 ttl 300
192.168.4.30:5353: Additional <UNKNOWN> MyQ-35E.local. rclass 0x8001 ttl 30
```
Where localhost is `192.168.4.115`

