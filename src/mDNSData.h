#ifndef SUBA_MDNS_TYPES
#define SUBA_MDNS_TYPES

#include "utils.h"


// using BufferType = char; // may change by platform, posix needs char
// using Buffer = std::vector<BufferType>;

/////////////////////////////////////////////////////////////////////////////////
// DATA STRUCTURES
/////////////////////////////////////////////////////////////////////////////////

// DNS Header structure
struct DNSHeader {
  uint16_t id; // Identifier
  uint16_t flags; // DNS flags
  uint16_t qdCount; // Number of questions
  uint16_t anCount; // Number of answers
  uint16_t nsCount; // Number of authority records
  uint16_t arCount; // Number of additional records

  enum Type {
    QUESTION = 0,
    ANSWER = 1,
    AUTHORITY = 2,
    ADDITIONAL = 4,
  };
  static const std::string& typeLookup( DNSHeader::Type t ) {
    static const std::string unknown = "unknown";
    static const std::map<DNSHeader::Type,std::string> l = {
      {Type::QUESTION, "QUESTION"},
      {Type::ANSWER, "ANSWER"},
      {Type::AUTHORITY, "AUTHORITY"},
      {Type::ADDITIONAL, "ADDITIONAL"},
    };
    return l.find( t ) != l.end() ? l.find( t )->second : unknown;
  }

  // Raw mDNS message callback type
  using Callback = std::function<void(const std::string& sender_ip, const char* buffer, uint16_t buffer_size)>;
  
  // a default callback that does nothing
  static void nop_cb(const std::string& sender_ip, const char* buffer, uint16_t buffer_size) {}

  DNSHeader() : id(0), flags(0), qdCount(0), anCount(0), nsCount(0), arCount(0) {}
};

// DNS Question structure
struct DNSQuestion {
  std::string qName; // Query name
  uint16_t qType; // Query type
  uint16_t qClass; // Query class

  using Callback = std::function<void(const std::string& sender_ip, const std::string& name, uint16_t type, uint16_t cls, bool flushbit, const char* buffer, uint16_t buffer_size, int pos)>;

  // a default callback that does nothing
  static void nop_cb(const std::string& sender_ip, const std::string& name, uint16_t type, uint16_t cls, bool flushbit, const char* buffer, uint16_t buffer_size, int pos) {}

  // https://en.wikipedia.org/wiki/List_of_DNS_record_types
  enum Type {
    A = 1,
    PTR = 12,
    TXT = 16,
    AAAA = 28, 
    SRV = 33,
    OPT = 41,
    NSEC = 47,
    ANY = 255,
    TYPE_UNKNOWN = -1
  };
  static const std::string& typeLookup( int64_t t ) {
    static const std::string unknown = "unknown";
    static const std::map<int64_t,std::string> l = {
      {Type::A, "A"},
      {Type::PTR, "PTR"},
      {Type::TXT, "TXT"},
      {Type::SRV, "SRV"},
      {Type::OPT, "OPT"},
      {Type::NSEC, "NSEC"},
      {Type::AAAA, "AAAA"}
    };
    return l.find( t ) != l.end() ? l.find( t )->second : unknown;
  }
  static const Type typeLookup( const std::string& t ) {
    static const std::map<std::string, Type> l = {
      {"A", Type::A},
      {"PTR", Type::PTR},
      {"TXT", Type::TXT},
      {"SRV", Type::SRV},
      {"OPT", Type::OPT},
      {"NSEC", Type::NSEC},
      {"AAAA", Type::AAAA}
    };
    return l.find( t ) != l.end() ? l.find( t )->second : Type::TYPE_UNKNOWN;
  }

  enum Class {
    UNKNOWN = 0, // 0x0000 - assignment requires an IETF Standards Action.
    IN = 1, // 0x0001 - Internet (IN).
    AVAILABLE = 2, // 0x0002 - available for assignment by IETF Consensus as a data CLASS.
    CH = 3, // 0x0003 - Chaos (CH) [Moon 1981].
    HS = 4, // 0x0004 - Hesiod (HS) [Dyer 1987].
    ANY_CLASS = 255,
  };
  static const std::string& classLookup( int64_t t ) {
    static const std::string unknown = "unknown";
    static const std::map<int64_t,std::string> l = {
      {Class::UNKNOWN, "UNKNOWN"},
      {Class::IN, "IN"},
      {Class::AVAILABLE, "AVAILABLE"},
      {Class::HS, "HS"},
      {Class::CH, "CH"},
      {Class::ANY_CLASS, "ANY"},
    };
    return (l.find( t ) != l.end()) ? l.find( t )->second : unknown;
  }

  DNSQuestion(const std::string& name, uint16_t type, uint16_t cls=Class::IN)
    : qName(name), qType(type), qClass(cls) {}
};

// DNS Resource Record structure
struct DNSResourceRecord {
  std::string rName; // Resource name
  uint16_t rType; // Resource type
  uint16_t rClass; // Resource class
  uint32_t ttl; // Time to live
  std::vector<uint8_t> rData; // Resource data

  using Callback = std::function<void(const std::string& sender_ip, DNSHeader::Type msg_type, const std::string& name, uint16_t type, uint16_t cls, bool flushbit, uint32_t ttl, std::vector<uint8_t>& data, const char* buffer, uint16_t buffer_size, int pos)>;

  // a default callback that does nothing
  void nop_cb(const std::string& sender_ip, DNSHeader::Type msg_type, const std::string& name, uint16_t type, uint16_t cls, bool flushbit, uint32_t ttl, std::vector<uint8_t>& data, const char* buffer, uint16_t buffer_size, int pos) {}

  DNSResourceRecord(const std::string& name, uint16_t type, uint16_t cls, uint32_t ttlVal, const std::vector<uint8_t>& data)
    : rName(name), rType(type), rClass(cls), ttl(ttlVal), rData(data) {}
};


//////////////////////////////////////////////////////////////////////////
// PARSING
//////////////////////////////////////////////////////////////////////////

template <typename T>
std::string parseDomainName(const T* buffer, int& pos, int length) {
  std::string name;
  while (pos < length) {
    unsigned char len = buffer[pos];
    if (len == 0) {
      pos++;
      break;
    }
    if ((len & 0xC0) == 0xC0) {
      // Pointer to another part of the packet
      int offset = ((len & 0x3F) << 8) | (unsigned char)buffer[pos + 1];
      pos += 2;
      int oldPos = pos;
      pos = offset;
      name += parseDomainName(buffer, pos, length);
      pos = oldPos;
      break;
    } else {
      pos++;
      name.append(buffer + pos, len);
      pos += len;
      name += '.';
    }
  }
  return name;
}

template <typename T>
void parseMDNSQuestion(const T* buffer, int& pos, int length, const std::string& sender_ip, DNSQuestion::Callback cb) {
  if (length <= pos) {
    fprintf( stderr, "[parseMDNSQuestion] Invalid mDNS packet (length:%d pos:%d).\n", length, pos );
    return;
  }

  // Parse the question name
  std::string name = parseDomainName(buffer, pos, length);

  // Parse the question type
  uint16_t qtype = ntohs(*(uint16_t*)&buffer[pos]);
  pos += 2;

  // Parse the question class
  uint16_t qclass = ntohs(*(uint16_t*)&buffer[pos]);
  uint16_t qclass_without_flushbit = qclass & (~0x8000);
  bool flushbit = (qclass & 0x8000) != 0;
  pos += 2;

  //printf( "  Name: %s\n", name.c_str() );
  //printf( "  Type:  0x%04x, %d, %s\n", (uint16_t)qtype, (uint16_t)qtype, DNSQuestion::typeLookup( qtype ).c_str() );
  //printf( "  Class: 0x%04x, %d, %s%s\n", (uint16_t)qclass, (uint16_t)qclass_without_flushbit, DNSQuestion::classLookup( qclass_without_flushbit ).c_str(), flushbit ? " +FLUSHBIT" : "" );

  cb( sender_ip, name, qtype, qclass_without_flushbit, flushbit, buffer, length, pos );
}

template <typename T>
void parseMDNSRecord(const T* buffer, int& pos, int length, const std::string& sender_ip, DNSResourceRecord::Callback cb, DNSHeader::Type msg_type) {
  if (length <= pos) {
    fprintf( stderr, "[parseMDNSRecord] Invalid mDNS packet (length:%d pos:%d).\n", length, pos );
    return;
  }
  std::string name = parseDomainName(buffer, pos, length);
  uint16_t rtype = ntohs(*(uint16_t*)&buffer[pos]);
  pos += 2;
  uint16_t rclass = ntohs(*(uint16_t*)&buffer[pos]);
  uint16_t rclass_without_flushbit = rclass & (~0x8000);
  bool flushbit = (rclass & 0x8000) != 0;
  pos += 2;
  uint32_t ttl = ntohl(*(uint32_t*)&buffer[pos]);
  pos += 4;
  uint16_t rdlength = ntohs(*(uint16_t*)&buffer[pos]);
  pos += 2;
  std::vector<uint8_t> rdata( &buffer[pos], &buffer[pos+rdlength] );

  // printf( "  Name: %s\n", name.c_str() );
  // printf( "  Type:  0x%04x, %d, %s\n", (uint16_t)rtype, (uint16_t)rtype, DNSQuestion::typeLookup( rtype ).c_str() );
  // printf( "  Class: 0x%04x, %d, %s%s\n", (uint16_t)rclass, (uint16_t)rclass_without_flushbit, DNSQuestion::classLookup( rclass_without_flushbit ).c_str(), flushbit ? " +FLUSHBIT" : "" );
  // printf( "  TTL: %d\n", (uint32_t)ttl );
  // printf( "  Data length: %d\n", (uint16_t)rdlength );

  int rdstart = pos; // Store the start position of RDATA
  cb( sender_ip, msg_type, name, rtype, rclass_without_flushbit, flushbit, ttl, rdata, buffer, length, pos );

/*
  switch (rtype) {
    case DNSQuestion::Type::A:
      printf( "  Address: %03d.%03d.%03d.%03d\n", (uint8_t)buffer[pos], (uint8_t)buffer[pos+1], (uint8_t)buffer[pos+2], (uint8_t)buffer[pos+3] );
      break;
    case DNSQuestion::Type::TXT:
      printf( "  TXT Data: %s\n", std::string(buffer + pos, rdlength).c_str() );
      if (rdlength < 100)
        hexDump( &buffer[pos], rdlength );
      else
        printf( "    - %d bytes (not showing, too long)\n", rdlength );
      break;
    case DNSQuestion::Type::PTR: {
      //int temp_pos = pos;
      std::string ptrname = parseDomainName(buffer, pos, length);
      printf( "  PTR Name: %s\n", ptrname.c_str() );
      break;
    }
    case DNSQuestion::Type::AAAA:
      //char addr_str[INET6_ADDRSTRLEN];
      //inet_ntop(AF_INET6, &buffer[pos], addr_str, sizeof(addr_str));
      //std::cout << "  Address: " << addr_str << std::endl;
      printf( "  Address AAAA ipv6: ???\n" );
      break;
    case DNSQuestion::Type::SRV: {
      uint16_t priority = ntohs(*(uint16_t*)&buffer[pos]);
      pos += 2;
      uint16_t weight = ntohs(*(uint16_t*)&buffer[pos]);
      pos += 2;
      uint16_t port = ntohs(*(uint16_t*)&buffer[pos]);
      pos += 2;
      //int temp_pos = pos + 6;
      std::string target = parseDomainName(buffer, pos, length);
      printf( "  Priority: %u\n", priority );
      printf( "  Weight: %u\n", weight );
      printf( "  Port: %u\n", port );
      printf( "  Target: %s\n", target.c_str() );
      break;
    }
    case DNSQuestion::Type::OPT: {
      // OPT record is typically used for EDNS0 options and may contain multiple options
      // For simplicity, we print the raw data
      printf( "  OPT Data: " );
      if (rdlength < 100)
        hexDump( &buffer[pos], rdlength );
      else
        printf( "    - %d bytes (not showing, too long)\n", rdlength );
      break;
    }
    case DNSQuestion::Type::NSEC: {
      //int temp_pos = pos;
      std::string nextDomainName = parseDomainName(buffer, pos, length);
      printf( "  Next Domain Name: %s\n", nextDomainName.c_str() );
      printf( "  Type Bitmaps:\n" );
      if (rdlength < 100)
        hexDump( &buffer[pos], rdlength );
      else
        printf( "    - %d bytes (not showing, too long)\n", rdlength );
      break;
    }
    case DNSQuestion::Type::ANY: {
      // ANY is a request for all records, so data parsing depends on response
      printf( "  ANY Data: " );
      if (rdlength < 100)
        hexDump( &buffer[pos], rdlength );
      else
        printf( "    - %d bytes (not showing, too long)\n", rdlength );
      break;
    }
    default:
      printf( "  Raw data: \n" );
      if (rdlength < 100)
        hexDump( &buffer[pos], rdlength );
      else
        printf( "    - %d bytes (not showing, too long)\n", rdlength );
  }
*/
  pos = rdstart + rdlength;
}

template <typename T>
void parseMDNSPacket(const T* buffer, int& pos, int length, const std::string& sender_ip, DNSHeader::Callback cb, DNSQuestion::Callback qCb, DNSResourceRecord::Callback rCb ) {
    cb( sender_ip, buffer, length );

    if (length < pos) {
      fprintf( stderr, "[parseMDNSPacket] Invalid mDNS packet (length:%d pos:%d).\n", length, pos );
      return;
    }

    uint16_t id = ntohs(*(uint16_t*)&buffer[0]);
    uint16_t flags = ntohs(*(uint16_t*)&buffer[2]);
    uint16_t qdcount = ntohs(*(uint16_t*)&buffer[4]);
    uint16_t ancount = ntohs(*(uint16_t*)&buffer[6]);
    uint16_t nscount = ntohs(*(uint16_t*)&buffer[8]);
    uint16_t arcount = ntohs(*(uint16_t*)&buffer[10]);
    // printf( "DNSHeader\n" );
    // printf( "- id %d\n", id );
    // printf( "- flags 0x%x\n", flags );
    // printf( "- qdCount %d (questions)\n", qdcount );
    // printf( "- anCount %d (answer)\n", ancount );
    // printf( "- nsCount %d (authority)\n", nscount );
    // printf( "- arCount %d (additional)\n", arcount );
    pos += 12;

    // if (0 < qdcount) printf( "Questions[%u]:\n", qdcount );
    for (int x = 0; x < qdcount; ++x) {
      parseMDNSQuestion( buffer, pos, length, sender_ip, qCb );
      // printf( "\n" );
    }

    // if (0 < ancount) printf( "Answers[%u]:\n", ancount );
    for (int i = 0; i < ancount; i++) {
      parseMDNSRecord(buffer, pos, length, sender_ip, rCb, DNSHeader::Type::ANSWER);
      // printf( "\n" );
    }

    // if (0 < nscount) printf( "Authorities[%u]:\n", nscount );
    for (int i = 0; i < nscount; i++) {
      parseMDNSRecord(buffer, pos, length, sender_ip, rCb, DNSHeader::Type::AUTHORITY);
      // printf( "\n" );
    }

    // if (0 < arcount) printf( "Additional records[%u]:\n", arcount );
    for (int i = 0; i < arcount; i++) {
      parseMDNSRecord(buffer, pos, length, sender_ip, rCb, DNSHeader::Type::ADDITIONAL);
      // printf( "\n" );
    }
}




//////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION
//////////////////////////////////////////////////////////////////////////////


template <typename T, typename TH>
void append( std::vector<T>& b, TH thing ) {
  b.insert(b.end(), reinterpret_cast<T*>(&thing), reinterpret_cast<T*>(&thing) + sizeof(thing));
}

// Function to convert a domain name to the DNS format
template <typename T>
void appendDomainName(std::vector<T>& buffer, const std::string& domain) {
  size_t pos = 0, start = 0;
  while ((pos = domain.find('.', start)) != std::string::npos) {
    buffer.push_back(static_cast<T>(pos - start));
    buffer.insert(buffer.end(), domain.begin() + start, domain.begin() + pos);
    start = pos + 1;
  }
  buffer.push_back(static_cast<T>(domain.size() - start));
  buffer.insert(buffer.end(), domain.begin() + start, domain.end());
  buffer.push_back(0); // Null terminator
}

// construct the mDNS query message
// use the result with:   buffer.data(), buffer.size()
template <typename T>
std::vector<T> makeQuestionBuffer( std::string resource/* = "mantis.local"*/, DNSQuestion::Type t) {
  DNSHeader header;
  header.qdCount = htons(1); // One question

  DNSQuestion question(resource.c_str(), t); // Query for an A record

  std::vector<T> buffer;
  append( buffer, header );
  appendDomainName( buffer, question.qName );
  append( buffer, htons(question.qType) );
  append( buffer, htons(question.qClass) );
  return buffer;
}

// mDNS response message
// use the result with:   buffer.data(), buffer.size()
template <typename T>
std::vector<T> makeAnswerBuffer( std::string resource /* = "mantis.local"*/, DNSQuestion::Type t) {
  DNSHeader header;
  header.anCount = htons(1); // One answer

  std::vector<uint8_t> ipAddr = {192, 168, 4, 114}; // Example IP address
  DNSResourceRecord answer(resource.c_str(), t, DNSQuestion::Class::IN, 120, ipAddr);

  std::vector<T> buffer;
  append( buffer, header );
  appendDomainName(buffer, answer.rName);
  append( buffer, htons(answer.rType) );
  append( buffer, htons(answer.rClass) );
  append( buffer, htonl(answer.ttl) );
  append( buffer, htons(answer.rData.size()) );
  buffer.insert(buffer.end(), answer.rData.begin(), answer.rData.end());
  return buffer;
}


#endif
