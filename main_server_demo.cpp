#include <iostream>
#include "Server.h"

int main(int argc, const char * argv[])
{

	ServerMaster s;

	s.server_init();

	s.listen_negotiate();

	while(1);

   return 0;
}


