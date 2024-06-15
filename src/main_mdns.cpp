#include <thread>
#include "mDNS.h"
#include "mDNSTestData.h"

// command line options:
struct CommandLineOptions {
  // standard args
  std::string processname;
  std::vector<std::string> args;
  bool VERBOSE=false;
  bool verbose_mdns=false;

  ////////////////////////////////////////////////////////////////////
  // custom args (add a new parse conditional to handle the --xxxx ):
  const std::string service_name_default = "_services._dns-sd._udp.local";
  std::string service_name = service_name_default;
  std::string ip_filter = "";
  int test=-1;
  bool query=false;
  bool answer=false;
  DNSQuestion::Type type=DNSQuestion::PTR;
  ////////////////////////////////////////////////////////////////////

  void usage() {
    printf( "%s - command line script to create a .sfz sampler instrument bank\n", processname.c_str() );
    printf( "Usage:\n" );
    //printf( "%s <arg>         (pass an arg - NOT USED)\n", processname.c_str() );
    printf( "%s --help        (this help)\n", processname.c_str() );
    printf( "%s --verbose     (output verbose information)\n", processname.c_str() );
    printf( "%s --verbose_mdns(output verbose mDNS messages)\n", processname.c_str() );
    printf( "%s --ip          (output filtered by ip)\n", processname.c_str() );
    printf( "%s --query       (make a query using the service --name)\n", processname.c_str() );
    printf( "%s   --name <name> (set the service name to query)\n", processname.c_str() );
    printf( "%s   --services    (set the name to _services)\n", processname.c_str() );
    printf( "%s   --suba        (set the name to _subachat)\n", processname.c_str() );
    printf( "%s --answer      (answer a question about the service --name)\n", processname.c_str() );
    printf( "%s   --type    (the type to answer with: PTR (default))\n", processname.c_str() );
    printf( "%s --test x      (run test 0..n)\n", processname.c_str() );
    printf( "\n" );
  };
  
  // parse the options
  void parse_args( int argc, char* argv[] ) {
    processname = argv[0];

    /////////////////////////////////////
    // scan command line args:
    const std::vector<std::string> ARGV(argv + 1, argv + argc); // 1st 1 is process name...
    const int ARGC = ARGV.size();
    const int non_flag_args_required = 0;
    int non_flag_args = 0;
    for (int i = 0; i < ARGC; ++i) {
      //printf( "ARGV[%d]=%s\n", i, ARGV[i].c_str() );
      if (ARGV[i] == "--help") {
        usage();
        exit( -1 );
      }
      if (ARGV[i] == "--verbose") {
        VERBOSE=true;
        continue;
      }
      if (ARGV[i] == "--ip") {
        i+=1;
        ip_filter=ARGV[i];
        VERBOSE && printf( "Parsing Args: setting ip_filter=%s\n", ip_filter.c_str() );
        continue;
      }
      if (ARGV[i] == "--verbose_mdns") {
        verbose_mdns=true;
        VERBOSE && printf( "Parsing Args: setting verbose_mdns=%d\n", verbose_mdns );
        continue;
      }
      if (ARGV[i] == "--query" || ARGV[i] == "-q") {
        query=true;
        VERBOSE && printf( "Parsing Args: setting query=%d\n", query );
        continue;
      }
      if (ARGV[i] == "--answer" || ARGV[i] == "-a") {
        answer=true;
        VERBOSE && printf( "Parsing Args: setting answer=%d\n", answer );
        continue;
      }
      if (ARGV[i] == "--name") {
        i+=1;
        service_name=ARGV[i];
        VERBOSE && printf( "Parsing Args: setting service_name=%s\n", service_name.c_str() );
        continue;
      }
      if (ARGV[i] == "--services") {
        service_name="_services._dns-sd._udp.local";
        VERBOSE && printf( "Parsing Args: setting service_name=%s\n", service_name.c_str() );
        continue;
      }
      if (ARGV[i] == "--suba") {
        service_name="_suBachat._udp.local";
        VERBOSE && printf( "Parsing Args: setting service_name=%s\n", service_name.c_str() );
        continue;
      }
      if (ARGV[i] == "--type") {
        i+=1;
        type = DNSQuestion::typeLookup( ARGV[i] );
        VERBOSE && printf( "Parsing Args: setting type=%d %s\n", type, ARGV[i].c_str() );
        continue;
      }
      if (ARGV[i] == "--test") {
        i+=1;
        test=atoi( ARGV[i].c_str() );
        VERBOSE && printf( "Parsing Args: setting test=%d\n", test );
        continue;
      }
      // example of parsing the next arg, dont delete
      // if (ARGV[i] == "--prefix") {
      //   i+=1;
      //   prefix=ARGV[i];
      //   prefix.replace( /\/+$/, '' ); // remove trailing slash if present
      //   VERBOSE && printf( "Parsing Args: setting prefix=%s\n", prefix.c_str() );
      //   continue;
      // }
      if (ARGV[i].substr(0,2) == "--") {
        printf( "Unknown option %s\n", ARGV[i].c_str() );
        exit(-1);
      }

      args.push_back( ARGV[i] );
      VERBOSE && printf( "Parsing Args: argument #${non_flag_args}: \"${%s}\"\n", ARGV[i].c_str() );
      non_flag_args += 1;
    }

    // output help if they're getting it wrong...
    if (non_flag_args_required != 0 && (ARGC == 0 || !(non_flag_args >= non_flag_args_required))) {
      (ARGC > 0) && printf( "Expected %d args, but only got %d\n", non_flag_args_required, non_flag_args );
      usage();
      exit( -1 );
    }
  }
};
///////////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] ) {
  CommandLineOptions opt;
  opt.parse_args( argc, argv );
  // if (!opt. && !opt.suba && opt.test < 0) {
  //   opt.usage();
  //   printf( "ERROR: no option given\n" );
  //   exit(-1);
  // }

  printf( "mDNS example\n" );
  mDNS transport;

  if (0 <= opt.test) {
    // interactive tests (run it and read & verify):
    int it;
    it = 0;
    switch (opt.test) {
      case 0: parseMDNSPacket( (const char*)testdata_1answer_4additional, it, sizeof( testdata_1answer_4additional ), "tst.tst.tst.tst.", DNSHeader::nop_cb, transport.printf_qCb, transport.printf_rCb ); break;
      case 1: parseMDNSPacket( (const char*)testdata_9answer_5additional, it, sizeof( testdata_9answer_5additional ), "tst.tst.tst.tst.", DNSHeader::nop_cb, transport.printf_qCb, transport.printf_rCb ); break;
      case 2: parseMDNSPacket( (const char*)testdata_1question, it, sizeof( testdata_1question ), "tst.tst.tst.tst.", DNSHeader::nop_cb, transport.printf_qCb, transport.printf_rCb ); break;
      case 3: parseMDNSPacket( (const char*)testdata_3question_2answer_1additional, it, sizeof( testdata_3question_2answer_1additional ), "tst.tst.tst.tst.", DNSHeader::nop_cb, transport.printf_qCb, transport.printf_rCb ); break;
      case 4: parseMDNSPacket( (const char*)testdata_4answer_7additional, it, sizeof( testdata_4answer_7additional ), "tst.tst.tst.tst.", DNSHeader::nop_cb, transport.printf_qCb, transport.printf_rCb ); break;
      default: printf( "Unknown test\n" );
    }
    exit(-1);
  }

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

  if (!opt.verbose_mdns) {
    transport.rawCallbacks.clear();
    transport.questionCallbacks.clear();
    transport.recordCallbacks.clear();

    // add a stdout handler for questions
    transport.questionCallbacks.push_back( [&opt]( const std::string& sender_ip, const std::string& name, uint16_t qtype, uint16_t qclass, bool flushbit, const char* buffer, uint16_t buffer_size, int pos ) {
      //printf( "%s %s\n", opt.service_name.c_str(), opt.ip_filter.c_str() );
      if (
        (opt.service_name == opt.service_name_default || name.find( opt.service_name ) != std::string::npos) &&
        (opt.ip_filter == "" || opt.ip_filter == sender_ip)
      )
        printf( "%-36s [%-10s] \"%s\" Type[0x%04x, %d, %s] Class[0x%04x, %d, %s%s]\n",
          sender_ip.c_str(),
          DNSHeader::typeLookup( DNSHeader::Type::QUESTION ).c_str(),
          name.c_str(),
          (uint16_t)qtype, (uint16_t)qtype, DNSQuestion::typeLookup( qtype ).c_str(),
          (uint16_t)qclass, (uint16_t)qclass, DNSQuestion::classLookup( qclass ).c_str(), flushbit ? " +FLUSHBIT" : ""
        );
    });

    // add a stdout handler for records
    transport.recordCallbacks.push_back( [&opt]( const std::string& sender_ip, DNSHeader::Type msg_type, const std::string& name, uint16_t rtype, uint16_t rclass, bool flushbit, uint32_t ttl, std::vector<uint8_t>& data, const char* buffer, uint16_t buffer_size, int pos ) {
      if (
        (opt.service_name == opt.service_name_default || name.find( opt.service_name ) != std::string::npos) &&
        (opt.ip_filter == "" || opt.ip_filter == sender_ip)
      )
        printf( "%-36s [%-10s] \"%s\" Type[0x%04x, %d, %s] Class[0x%04x, %d, %s%s] ttl:%d\n",
          sender_ip.c_str(),
          DNSHeader::typeLookup( msg_type ).c_str(),
          name.c_str(),
          (uint16_t)rtype, (uint16_t)rtype, DNSQuestion::typeLookup( rtype ).c_str(),
          (uint16_t)rclass, (uint16_t)rclass, DNSQuestion::classLookup( rclass ).c_str(), flushbit ? " +FLUSHBIT" : "",
          ttl
        );
    });
  }

  if (opt.answer) {
    transport.questionCallbacks.push_back( [&opt, &transport]( const std::string& sender_ip, const std::string& name, uint16_t qtype, uint16_t qclass, bool flushbit, const char* buffer, uint16_t buffer_size, int pos ) {
      if (qtype == DNSQuestion::PTR && name.find( opt.service_name ) != std::string::npos) {
        printf( "reply to the service question for %s!\n", opt.service_name.c_str() );
        std::vector<char> send_buf = makeQuestionBuffer<char>( opt.service_name, opt.type );
        transport.send( send_buf.data(), send_buf.size() );
      }
    });
  }

  // send a query out later...
  if (opt.query) {
    std::thread send_thread( [&transport, &opt](){
      sleep( 1 );
      printf( "send a 'query' for name:%s\n", opt.service_name.c_str() );
      //transport.send( query, sizeof(query) );                   // 192.168.4.114:58749: Question PTR _services._dns-sd._udp.local. rclass 0x1 ttl 0
      std::vector<char> send_buf = makeQuestionBuffer<char>( opt.service_name, DNSQuestion::PTR );
      transport.send( send_buf.data(), send_buf.size() );       // 192.168.4.114:56887: Question PTR  _suBachat._udp.local. rclass 0x1 ttl 0
      sleep( 4 );
    });
    send_thread.join();
  }

  // std::vector<char> resp_buf = makeAnswerBuffer<char>( "_suBachat._udp.local.", DNSQuestion::PTR );
  // transport.send( resp_buf.data(), resp_buf.size() );       // 192.168.4.114:51107: Answer A _suBachat._udp.local. rclass 0x1 ttl 120

  // wait for thread to end (hint, it never will if recv() is a while(1) )
  t.join();

  return 0;
}
