#include <thread>
#include "UDP.h"

int main() {
  printf( "UDP example\n" );

  UDP transport;

  // create a listener
  std::function func = [&transport](){
      transport.recv();
  };
  std::thread t( func );

  // send some test messages:
  transport.send( "hi", strlen( "hi" ) );
  transport.send( "bye", strlen( "bye" ) );
  transport.send( "fly", strlen( "fly" ) );

  // wait for thread to end (hint, it never will if recv() is a while(1) )
  t.join();

  return 0;
}
