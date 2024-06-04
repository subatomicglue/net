#include <thread>
#include "mDNS.h"
#include "testdata.h"

//#define UNIT_TEST 1

int main() {
  printf( "mDNS example\n" );
  #ifdef UNIT_TEST
  // unit tests:
  int it;
  it = 0;
  parseMDNSPacket( (const char*)testdata_1answer_4additional, it, sizeof( testdata_1question ) );
  it = 0;
  parseMDNSPacket( (const char*)testdata_9answer_5additional, it, sizeof( testdata_1question ) );
  // it = 0;
  // parseMDNSPacket( (const char*)testdata_1question, it, sizeof( testdata_1question ) );
  // it = 0;
  // parseMDNSPacket( (const char*)testdata_3question_2answer_1additional, it, sizeof( testdata_3question_2answer_1additional ) );
  // it = 0;
  // parseMDNSPacket( (const char*)testdata_4answer_7additional, it, sizeof( testdata_4answer_7additional ) );
  exit(-1);
  #endif

  mDNS transport;

  // create a listener
  std::thread t( [&transport](){
      transport.recv();
  });

  sleep(1);

  // char query[] = "\x00\x00" // Transaction ID
  //                  "\x00\x00" // Flags
  //                  "\x00\x01" // Questions
  //                  "\x00\x00" // Answer RRs
  //                  "\x00\x00" // Authority RRs
  //                  "\x00\x00" // Additional RRs
  //                  "\x09_services\x07_dns-sd\x04_udp\x05local\x00" // _services._dns-sd._udp.local
  //                  "\x00\x0C" // PTR type
  //                  "\x00\x01"; // Class IN

  // send some test messages:
  //transport.send( query, sizeof(query) );                   // 192.168.4.114:58749: Question PTR _services._dns-sd._udp.local. rclass 0x1 ttl 0
  std::vector<char> send_buf = makeQuestionBuffer<char>( "_suBachat._udp.local", DNSQuestion::PTR );
  transport.send( send_buf.data(), send_buf.size() );       // 192.168.4.114:56887: Question PTR  _suBachat._udp.local. rclass 0x1 ttl 0
  // sleep( 4 );
  // std::vector<char> resp_buf = makeAnswerBuffer<char>( "_suBachat._udp.local.", DNSQuestion::PTR );
  // transport.send( resp_buf.data(), resp_buf.size() );       // 192.168.4.114:51107: Answer A _suBachat._udp.local. rclass 0x1 ttl 120

  // wait for thread to end (hint, it never will if recv() is a while(1) )
  t.join();

  return 0;
}
