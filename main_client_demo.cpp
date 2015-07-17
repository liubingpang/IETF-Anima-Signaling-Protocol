#include "Client.h"
#include <unistd.h>

int main(int argc, const char * argv[])
{
    // Client
    Client c;
    char test[MAXSTRINGLENGTH] = "test1";

	c.discover();

	//afte discover , negotiate or synchronize
	c.negotiate(test);

//    c.synchronize(test);
    while(1);


    return 0;


}
