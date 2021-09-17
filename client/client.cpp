#include "client.h"
#include <iostream>
#include <AMQPcpp.h>


using namespace std;

// define static member here
Client *Client::_inst = NULL;

// declare constructor
Client::Client(){
    port=9999;
    sock=0;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    try {
            amqp=new AMQP("AK.Tanla:Welcome#$123@localhost:5672//");

        } 
        catch (AMQPException e) {
            std::cout << e.getMessage() << std::endl;
        }
}
// getter and setters

int Client::getSock() const{
    return sock; 
}


// connection creator
int Client::createConnection(){
    {
            if ((sock = socket(AF_INET, SOCK_STREAM , 0)) < 0)
            {
                printf("\n Socket creation error \n");
                return -1;
            }
            
            // Convert IPv4 and IPv6 addresses from text to binary form
            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
            {
                printf("\nInvalid address/ Address not supported \n");
                return -1;
            }

            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                return -1;
            }

            return sock;
        }


        // creating connection with rabbitmq server 

        
}
Client* Client::getInstance(){
    if(_inst==NULL){
        _inst= new Client();
    }
    return _inst;

}

AMQP* Client::getAMQP() const{
    return amqp;
}


