#include "client.h"
#include <iostream>


using namespace std;

// define static member here
Client *Client::_inst = NULL;

// declare constructor
Client::Client(){
    port=9999;
    sock=0;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
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
}
Client* Client::getInstance(){
    if(_inst==NULL){
        _inst= new Client();
    }
    return _inst;

}


// 	// cout<<"Connection established \n";
// 	recv(sock, (char*)&msg, sizeof(msg), 0);
// 	cout<<msg<<endl;
// 	cout<<"Ready for sending new message "<<endl;
//     cout<<"Enter 1 to Login \n Enter 2 to SignUp if you are new User \n ";
//     int option;
//     while(1){
//         cout<<"Enter your Choice :: ";
//         cin>>option;
//         if(option==1 || option==2)
//             break;
//         else {
//             cout<<"invalid option Enter ur choice again \n";
//         }
//     }
//     switch(option){
//         case 1:
//     }
//     while(1)
//     {
//         cout << ">";
//         string data;
//         getline(cin, data);
//         memset(&msg, 0, sizeof(msg));//clear the buffer
//         strcpy(msg, data.c_str());
//         if(data == "exit")
//         {
//             send(sock, (char*)&msg, strlen(msg), 0);
//             break;
//         }
//         send(sock, (char*)&msg, strlen(msg), 0);
//         cout << "Awaiting server response..." << endl;
        
//         memset(&msg, 0, sizeof(msg));//clear the buffer
//         recv(sock, (char*)&msg, sizeof(msg), 0);

//         // showing message returned from server
//         cout << "Server: " << msg << endl;
//     }
//     cout << "********Session********" << endl;
//     cout << "Connection closed" << endl;
// 	return 0;
// }

// void  INThandler(int sig)
// {
//      char  c;

//      signal(sig, SIG_IGN);
//      printf("OUCH, did you hit Ctrl-C?\n"
//             "Do you really want to quit? [y/n] ");
//      c = getchar();
//      if (c == 'y' || c == 'Y'){
// 		 send(sock, (char*)"exit", strlen("exit"), 0);
// 		 exit(0);
// 	 }
          
//      else
//         signal(SIGINT, INThandler);
//         getchar(); // Get new line character
// }

