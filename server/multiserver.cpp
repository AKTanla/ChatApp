#include "multiserver.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include "../client/user.h"
#include <mysql/mysql.h>
#include <string.h>
#include <cstring>
#include <AMQPcpp.h>

using namespace std;

// define static member here
Server *Server::_inst =NULL;
// defining constructor
Server::Server(){
    // assigning port
    port =9999;
    master_socket =0;
    //type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );


	// for SQL server
	conn = mysql_init(NULL);
	host="localhost", user="root", pass="Welcome#$123", dbName="ChatSocket";
    sqlport=3306;
    *unix_socket==NULL;
	flag=0;
}


// get instance 



Server* Server::getInstance(){
    if(_inst==NULL){
        _inst=new Server();
    }
    return _inst;
}


// get create server
int Server::createServer(){
    
    int opt = 1;
    //initialise all client_socket[] to 0 so not checked
	
	for (int i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}
		
	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

    //set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

    //binding socket
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	cout<<"socket binding Successful"<<endl;

	// connect to sql database
	if(!mysql_real_connect(conn,host,user,pass,dbName,sqlport,unix_socket,flag))
	{
		fprintf(stderr,"Error: %s [%d]",mysql_error(conn),mysql_errno(conn));
		exit(1);
	}


	// creating rabbitmq server
	try {

		string loginString="AK.Tanla:Welcome#$123@localhost:5672//";
		amqp=new AMQP(loginString);		// all connect string


		ex = amqp->createExchange("ChatApp");
		ex->Declare("ChatApp", "direct",AMQP_DURABLE);

			

		ex->setHeader("Delivery-mode", 2);
		ex->setHeader("Content-type", "text/text");
		ex->setHeader("Content-encoding", "UTF-8");
														
						
	} catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}
	return 0;
}   
// listen function

void Server::listenServer(){
    //listening to port 

	int max_sd,sd,activity;
    if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

    // using select function for activity on the master socket;
    // here we will
    while(true)
	{
		//clear the socket set
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&errfds);
	
		//add master socket to set
		FD_SET(master_socket, &readfds);
		FD_SET(master_socket, &errfds);
		max_sd = master_socket;
		

		//add child sockets to set
		for (int  i = 0 ; i < max_clients ; i++)
		{
			//socket descriptor
			sd = client_socket[i];
				
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
				FD_SET( sd , &errfds);
				
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}
		
		
		
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select( max_sd + 1 , &readfds , &writefds , &errfds , NULL);
	
		if(activity>0){
			// socket is availble for read write operation
			//read fds
				processTheNewRequest();
			//err fds

			// write fds

		


		}else if(activity==0){
			// no connection are in communication request made 
			//that noneof the socket descriptors are ready 
			cout<<"Nothing on port"<<endl;

		}else{
			//connection failed ur application 
			cout<< "No connection Available"<<endl;
		}
		         
	}
}

// process new request

void Server::processTheNewRequest()
{
	// ckeck for new connection request
	char msgConnEst[100]="Connection Established";
    int addrlen = sizeof(address);

	if(FD_ISSET(master_socket,&readfds))
    {
		int nClientSocket=accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);

		if(nClientSocket>0)
        {
			// add client to arrayClient
			int i;
			for(i=0;i<max_clients;i++)
            {
				if(client_socket[i]==0)
                {
					client_socket[i]=nClientSocket;					
					break;
				}
            }

            if(i==max_clients)
            {
                cout<<"Maximum Client Limit Exceeded"<<endl;
                return;
			}

			client_socket_thread[nClientSocket]=thread(&Server::processNewClient,this,nClientSocket);
				if(client_socket_thread[nClientSocket].joinable())
                {
					client_socket_thread[nClientSocket].detach();
				}			
			
		}
	}
}


void Server::processNewClient(int nClientSocket)
{
	int nRet;
	char buff[2024]={0,};
	while(1){
		nRet=recv(nClientSocket,(char*)&buff,1024,0);
		if(nRet<0)
		{
			cout<<endl<<"Something wrong happened closing the connecction"<<endl;
			close(nClientSocket);
			for(int i=0;i<max_clients;i++){
				if(client_socket[i]==nClientSocket){
					
					client_socket[i]=0;
					break;	
				}
			}
			return;
		}
		else if(nRet==0){
			cout<<"Forced shutdown or something like this happened on client side"<<endl;
			close(nClientSocket);
			for(int i=0;i<max_clients;i++){
				if(client_socket[i]==nClientSocket){
					client_socket[i]=0;
					break;	
				}
			}
			auto it=onlineUser.find(nClientSocket);
			if(it!=onlineUser.end()){
				onlineUser.erase(it);
			}
			return;
		}
		else
		{
			vector<string> data;
			string T;  // declare string variables  

			stringstream X(buff); // X is an object of stringstream that references the S string  

			// use while loop to check the getline() function condition  
			while (getline(X, T, '|')) {  
				/* X represents to read the string from stringstream, T use for store the token string and, 
				'|' pipe represents to split the string where pipe is found. */  
				data.push_back(T);// store split string  
			}  
			cout<<data[0]<<endl;

			// logic here 
            //***************************************************************************************************			
			if(data[0]=="SIGNUP")
			{
				RegisterUser user;
				user.setFirstName(data[1]);
				user.setLastName(data[2]);
				user.setUserName(data[3]);
				user.setEmail(data[4]);
				user.setPassword(data[5]);
				string InsertUser="INSERT INTO user (user_firstName, user_lastName, user_name, user_email, user_password) VALUES('"+user.getFirstName()+"','"+user.getLastName()+"','"+user.getUserName()+"','"+user.getEmail()+"','"+user.getPassword()+"')";
				int queryStatus=mysql_query(conn,InsertUser.c_str());
				if(!queryStatus){
					cout<<"data inserted successfully"<<endl;
					send(nClientSocket,"SUCCESS",7,0);
					cout<<"************************************************************"<<endl;
					close(nClientSocket);
					for(int i=0;i<max_clients;i++){
						if(client_socket[i]==nClientSocket){
							
							client_socket[i]=0;
							break;	
						}
			        }
					return;
				}
				else{
					cout<<"unable to insert"<<endl;
					cout<<mysql_error(conn)<<endl;
					send(nClientSocket,mysql_error(conn),strlen(mysql_error(conn)),0);
					close(nClientSocket);
					for(int i=0;i<max_clients;i++){
						if(client_socket[i]==nClientSocket){
							
							client_socket[i]=0;
							break;	
						}
					}
					return;
				}
			    return;

			}
			else if (data[0]=="LOGIN")
			{
				cout<<"Login Operation"<<endl;
				int sendRes;
				string queryLogin="SELECT user_password FROM user WHERE user_email='"+data[1]+"'";
				int queryStatus=mysql_query(conn, queryLogin.c_str());
				cout<<queryStatus<<endl;
				if(!queryStatus)
				{
					MYSQL_RES *res;
					res =mysql_store_result(conn);
					MYSQL_ROW row;
					int numRow =mysql_num_rows(res);
					row = mysql_fetch_row(res); 
					if(numRow>0){
						if(row[0]==data[2])
						{
							mysql_free_result(res);
							cout<<"You are Valid user"<<endl;
							sendRes=send(nClientSocket,"SUCCESS",7,0);
							if(sendRes>0)
							{
								onlineUser[nClientSocket]=data[1];
								close(nClientSocket);
								for(int i=0;i<max_clients;i++)
								{
									if(client_socket[i]==nClientSocket)
									{
										
										client_socket[i]=0;
										break;	
									}
								}
								return;
							}
							else
							{
								cout<<"not able to send success message"<<endl;
								close(nClientSocket);
								for(int i=0;i<max_clients;i++)
								{
									if(client_socket[i]==nClientSocket)
									{
										
										client_socket[i]=0;
										break;	
									}
								}
							}
						}
						else
						{
				//*************** user not authorised *********************
							mysql_free_result(res);
							send(nClientSocket,mysql_error(conn),strlen(mysql_error(conn)),0);
							cout<<endl<<"Something wrong happened closing the connecction"<<endl;
							close(nClientSocket);
							for(int i=0;i<max_clients;i++){
								if(client_socket[i]==nClientSocket){
									
									client_socket[i]=0;
									break;	
								}
							}
							return;
						}
					}
					else{
			// here send to client that user is not avilable in databsae so register first*************
						char m[100]="NOT_REGISTERED";
						send(nClientSocket,&m,strlen(m),0);
						cout<<endl<<"USER NOT REGISTERED "<<endl;
						close(nClientSocket);
						for(int i=0;i<max_clients;i++){
							if(client_socket[i]==nClientSocket){
								
								client_socket[i]=0;
								break;	
							}
						}
						return;
					}
					
					
				}
				else
				{
					cout<<"Query not Succesful some error occured "<<endl;
					cout<<mysql_error(conn)<<endl;
					close(nClientSocket);
						for(int i=0;i<max_clients;i++){
							if(client_socket[i]==nClientSocket){
								
								client_socket[i]=0;
								break;	
							}
						}
					return;					
				}
			}// End of LOGIN SECTION ***********************************************************************
			else if(data[0]=="CHAT")
			{
				// This is the chat handling section of  server ********************************************
				cout<<buff<<endl;
				string checkForUserRegistration="SELECT * FROM user WHERE user_email='"+data[2]+"'";
				int queryStatus=mysql_query(conn, checkForUserRegistration.c_str());
				cout<<"chat query status"<<queryStatus<<endl;
				if(!queryStatus){
					MYSQL_RES *res;
					res =mysql_store_result(conn);
					MYSQL_ROW row;
					int numRow = mysql_num_rows(res);
					row = mysql_fetch_row(res); 
					if(numRow>0){
						mysql_free_result(res);
						try{
							cout<<"user is registered"<<endl;
							string dest=data[2];
							string messageTosend="";
							messageTosend=data[1]+"|"+data[3]+"|"+data[4];
							string queue = dest;
							AMQPQueue *qu = amqp->createQueue(queue);
							qu->Declare(queue,AMQP_DURABLE);
							qu->Bind( "ChatApp", queue);
							ex->Publish(  messageTosend, queue);
						}
						catch (AMQPException e) {
							std::cout << e.getMessage() << std::endl;
							
						}
					}else{
						try{
							cout<<"user is not registered "<<endl;
							string queue = data[1];
							AMQPQueue *qu = amqp->createQueue(queue);
							qu->Declare(queue,AMQP_DURABLE);
							qu->Bind( "ChatApp", queue);
							ex->Publish( "user not registered or not available on this chat app", queue);
						}catch (AMQPException e) {
							std::cout << e.getMessage() << std::endl;
							
						}
						
					}
				}
				else{
					cout<<"Query not Succesful Email not found or something else happened"<<endl;
					cout<<mysql_error(conn)<<endl;
					close(nClientSocket);
						for(int i=0;i<max_clients;i++){
							if(client_socket[i]==nClientSocket){
								
								client_socket[i]=0;
								break;	
							}
						}
					return;	
				}
				
				
               // chat section completed
			}//END OF CHAT SECTION ********************************************************************
			else if(data[0]=="LOGOUT"){
				for(auto it=onlineUser.begin();it!=onlineUser.end();it++){
					if((it->second)==data[1]){
						cout<<"user -"<< data[1] <<"-is loggedOut"<<endl;
						onlineUser.erase(it);
						break;	
					}

				}
				close(nClientSocket);
				for(int i=0;i<max_clients;i++){
					if(client_socket[i]==nClientSocket){
						
						client_socket[i]=0;
						break;	
					}
				}
				return;

			}		
			else 
			{
				cout<<"Invalid operation asked to perfornm";
				close(nClientSocket);
				for(int i=0;i<max_clients;i++){
					if(client_socket[i]==nClientSocket){
						
						client_socket[i]=0;
						break;	
					}
				}
				return;
			}
			
		}
		
	}
	
	return;
}


