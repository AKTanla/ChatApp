// Client side C/C++ program to demonstrate Socket programming
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <AMQPcpp.h>


using namespace std;



class Client{

    int sock;
    int port;
    struct sockaddr_in serv_addr;

    // for rabbitmq
    AMQP *amqp;
    AMQPQueue *messageQueue;

    Client(); //private constructor

    static Client* _inst;
    public:
        int getSock() const;
        AMQP* getAMQP() const;
        static Client* getInstance();
        int createConnection();

};

