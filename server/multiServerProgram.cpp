#include "multiserver.h"
#include <iostream>

using namespace std;


int main(){

    Server *server = Server::getInstance();
    int status=server->createServer();
    if(status==0){
        server->listenServer();
    }
    return 0;
}