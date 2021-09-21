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
#include <ctime>
#include <Poco/AutoPtr.h>
#include <Poco/Util/IniFileConfiguration.h>
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"


using namespace std;
using Poco::AutoPtr;
using Poco::Util::IniFileConfiguration;
using Poco::Logger;
using Poco::SimpleFileChannel;



// define static member here
Server *Server::_inst =NULL;
// defining constructor
Server::Server()
{

	AutoPtr<IniFileConfiguration> pConf(new IniFileConfiguration("../config.ini"));
    // assigning port
    port =pConf->getInt("ChatServer.serverport");
	// port =9999;
    master_socket =0;
    //type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );


	// for SQL server
    string hostString = pConf->getString("Database.host");
    sqlport = pConf->getInt("Database.port");
    string passString = pConf->getString("Database.password");
	string dbNameString =pConf->getString("Database.dbName");
	string userString =pConf->getString("Database.user");
	
	conn = mysql_init(NULL);
	host=(char*)&hostString[0];
	pass=(char*)&passString[0];
	user=(char*)&userString[0];
	dbName=(char*)&dbNameString[0];
    *unix_socket==NULL;
	flag=0;

	// logger for server
	pChannel= new SimpleFileChannel;
    pChannel->setProperty("path", "ChatServer.log");
    pChannel->setProperty("rotation", "4 K");
    Logger::root().setChannel(pChannel);
	
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
		logErr("socket Failed");
		exit(EXIT_FAILURE);
	}

    //set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		logErr("setSocketOpt");
		exit(EXIT_FAILURE);
	}

    //binding socket
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		logErr("bind failed");
		exit(EXIT_FAILURE);
	}

	cout<<"socket binding Successful"<<endl;
	logInfo("Socket Binding Successful");

	// connect to sql database
	if(!mysql_real_connect(conn,host,user,pass,dbName,sqlport,unix_socket,flag))
	{
		fprintf(stderr,"Error: %s [%d]",mysql_error(conn),mysql_errno(conn));
		logErr(mysql_error(conn));
		exit(1);
	}


	// creating rabbitmq server
	try {

		string loginString="AK.Tanla:Welcome#$123@localhost:5672//";
		amqp=new AMQP(loginString);		// all connect string

		// this exchange is for chatApp
		ex = amqp->createExchange("ChatApp");
		ex->Declare("ChatApp", "direct",AMQP_DURABLE);

		// this queue is to loginReport
		loginReport=amqp->createQueue("loginReport");
		loginReport->Declare("loginReport",AMQP_DURABLE);
		loginReport->Bind("ChatApp","loginReport");

		// this queue is for signUpReport
		signUpReport=amqp->createQueue("signUpReport");
		signUpReport->Declare("signUpReport",AMQP_DURABLE);
		signUpReport->Bind("ChatApp","signUpReport");

		// this queue is for sending message to database
		messageToDatabase=amqp->createQueue("messageToDatabase");
		messageToDatabase->Declare("messageToDatabase",AMQP_DURABLE);
		messageToDatabase->Bind("ChatApp","messageToDatabase"); 

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
		logErr("listen failed");
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
			logInfo("nothing on port ");

		}else{
			//connection failed ur application 
			cout<< "No connection Available"<<endl;
			logInfo("No connection available");
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
				logInfo("Maximum Client Limit Exceded");
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
			logErr("Something  wrong happened closing the connection");
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
			logInfo("Forced shutdown or something like this happened on client side");
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
					string timeStamp=getCurrentTime();
					string signUpReport=user.getEmail()+"|"+timeStamp;
					try{
						ex->Publish(signUpReport, "signUpReport");
					}
					catch (AMQPException e) {
						std::cout << e.getMessage() << std::endl;
						
					}
					cout<<"data inserted successfully"<<endl;
					send(nClientSocket,"SUCCESS",7,0);
					logInfo(user.getEmail()+"  Signed Up succesfully");
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
					logErr(user.getEmail()+"  Could not SignUp succesfully");
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
				string queryLogin="SELECT user_id, user_password FROM user WHERE user_email='"+data[1]+"'";
				int queryStatus=mysql_query(conn, queryLogin.c_str());
				if(!queryStatus)
				{
					MYSQL_RES *res;
					res =mysql_store_result(conn);
					MYSQL_ROW row;
					int numRow =mysql_num_rows(res);
					row = mysql_fetch_row(res); 
					if(numRow>0){
						if(row[1]==data[2])
						{
							
							cout<<"You are Valid user"<<endl;
							time_t rawtime;
							struct tm * timeinfo;
							char buffer[200];
							time (&rawtime);
							timeinfo = localtime(&rawtime);
							strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
							string timeStamp(buffer);
							string loginReport="INSERT INTO userLogin (logger_id,login_time) VALUES ("+string(row[0])+",'"+timeStamp+"')";
							mysql_free_result(res);
							try{
							ex->Publish(loginReport, "loginReport");
							}
							catch (AMQPException e) {
								std::cout << e.getMessage() << std::endl;
								
							}
							logInfo(data[1]+"  Signed In succesfully");
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
							string notAuthorised="Incorret User Name Or Password";
							logInfo(data[1]+"Incorrect User Name or Password");
							send(nClientSocket,(char*)&notAuthorised[0] ,sizeof(notAuthorised),0);
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
						logInfo(data[1]+" Is not Registered");
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
					logErr(mysql_error(conn));
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
			}// End of LOGIN SECTION ***********************************************************************
			else if(data[0]=="CHAT")
			{
				// This is the chat handling section of  server ********************************************
				string checkForUserRegistration="SELECT * FROM user WHERE user_email='"+data[2]+"'";
				int queryStatus=mysql_query(conn, checkForUserRegistration.c_str());
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
							string messageTosave=data[1]+"|"+data[2]+"|"+data[3]+"|"+data[4];
							ex->Publish(messageTosave,"messageToDatabase");
						}
						catch (AMQPException e) {
							std::cout << e.getMessage() << std::endl;
							
						}
						close(nClientSocket);
						for(int i=0;i<max_clients;i++){
							if(client_socket[i]==nClientSocket){
								
								client_socket[i]=0;
								break;	
							}
						}
						return;	
					}else{
						try{
							cout<<"user is not registered "<<endl;
							string queue = data[1];
							AMQPQueue *qu = amqp->createQueue(queue);
							qu->Declare(queue,AMQP_DURABLE);
							qu->Bind( "ChatApp", queue);
							ex->Publish( "user not registered or not available on this chat app", queue);
							logInfo(data[2]+" user not available ");
						}catch (AMQPException e) {
							std::cout << e.getMessage() << std::endl;
							
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
				}
				else{
					cout<<"Query not Succesful"<<endl;
					cout<<mysql_error(conn)<<endl;
					logErr("Query not Succesful"+string(mysql_error(conn)));
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
				logInfo(data[1]+"  Signed Out succesfully");
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

string getCurrentTime(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[200];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);
    string timeStamp(buffer);
    return timeStamp;
}

string getMessage(string message){
	return getCurrentTime()+" "+message;
}

void Server::logInfo(string message){
	
	Logger& logObject = Logger::get("ServerLogger"); // inherits root channel
	logObject.information("info -> "+getMessage(message));
}

void Server::logErr(string message){
	Logger& logObject = Logger::get("ServerLogger"); // inherits root channel
	logObject.error("error -> "+getMessage(message));
}

