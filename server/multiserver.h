//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include<thread>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h> //strlen
#include <unistd.h> //close
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macro
#include <map>
#include <sys/ioctl.h>
#include <mysql/mysql.h>
#include <AMQPcpp.h>
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"

using namespace std;
using Poco::Logger;
using Poco::SimpleFileChannel;
using Poco::AutoPtr;

 const int max_clients=64;

class Server{
    fd_set readfds, writefds, errfds;
    int master_socket;
    struct sockaddr_in address;
    int client_socket[max_clients];
    int port;
    static Server *_inst;
    map<int, thread> client_socket_thread;

    // for SQL

    MYSQL *conn;
    char *host, *user, *pass, *dbName;
    unsigned int sqlport;
    char *unix_socket;
    unsigned int flag;

    // for maintaining Logged in user
    map <int, string> onlineUser;

    // for rabbitmq 
    AMQPExchange * ex;
    AMQP *amqp;
    AMQPQueue *loginReport;
    AMQPQueue *signUpReport;
    AMQPQueue *messageToDatabase;


    // logger and channel

    AutoPtr<SimpleFileChannel> pChannel;
    // Logger& logger;
    

    Server();

    public:
        static Server* getInstance();
        int createServer();
        void listenServer();
        void processTheNewRequest();
        void processNewClient(int nClientSocket);
        int publishMessage(string message ); 
        void logInfo(string message);
        void logErr(string message);


};

string getCurrentTime();
string getMessage(string message);


