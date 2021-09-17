#include "client.h"
#include <iostream>
#include <ostream>
#include <string>
#include <cstring>
#include <string.h>
#include <stdio.h>
#include "user.h"
#include <boost/regex.hpp>
#include <thread>
#include <ctime>
#include <AMQPcpp.h>

using namespace std;
using namespace boost;

// global for all function can use that
LoginUser loginUser ;
RegisterUser user;

void saveToLocalStorage(){
    
}

void showOption(){
    cout<<"*****************************************************************"<<endl;
    cout<<"*            HERE ARE YOUR OPTIONS                              *"<<endl;
    cout<<"*            1. TO SIGN UP                                      *"<<endl;
    cout<<"*            2. TO SIGN IN                                      *"<<endl;
    cout<<"*            3. TO START CHAT                                   *"<<endl;
    cout<<"*            4. TO EXIT                                         *"<<endl;
    cout<<"*****************************************************************"<<endl;
}

bool emailValidation(string email){
    regex emailexp{"\\w+([\\.-]?\\w+)*@\\w+([\\.-]?\\w+)*(\\.\\w{2,3})+$"};
    bool x=regex_match(email, emailexp);
    return x;
}

RegisterUser showSignUpWindowandCollectData()
{
    string firstName;
    string lastName;
    string userName;
    string email;
    string password;
    RegisterUser newUser;
    cout<<"*****************************************************************"<<endl;
    cout<<"*                 =====SIGN UP=====                             *"<<endl;
    do{
        cout<<"*  FIRST NAME             : ";cin>>(firstName);    
    }while(firstName.empty());                 
    do{
        cout<<"*  LAST NAME              : ";cin>>(lastName);    
    }while(lastName.empty()); 
    do{
        cout<<"*  USER NAME              : ";cin>>(userName);    
    }while(userName.empty());
    do{
        cout<<"*  EMAIL                  : ";cin>>(email);    
    }while((email.empty()? true:!emailValidation(email)));
    do{
        cout<<"*  PASSWORD               : ";cin>>(password);    
    }while(password.empty());               
    cout<<"*****************************************************************"<<endl;
    
    newUser.setFirstName(firstName);
    newUser.setLastName(lastName);
    newUser.setUserName(userName);
    newUser.setEmail(email);
    newUser.setPassword(password);
    return newUser;
}

// int passValidate(string password){

// }
LoginUser showSignInWindowAndCollectData(){
    LoginUser user;
    string userEmail;
    string password;
    do{
        cout<<"*  EMAIL             : ";cin>>(userEmail);    
    }while((userEmail.empty()? true:!emailValidation(userEmail)));                 
    do{
        cout<<"*  PASSWORD          : ";cin>>(password);    
    }while(password.empty());

    user.setEmail(userEmail);
    user.setPassword(password);
    return user;
}

int onCancel(AMQPMessage * message ) {
	cout << "cancel tag="<< message->getDeliveryTag() << endl;
	return 0;
}

int  onMessage( AMQPMessage * message  ) {
	uint32_t j = 0;
    char * recBuf = message->getMessage(&j);
    if (recBuf){
        vector<string> data;
        string T;  // declare string variables  
        stringstream X(recBuf); // X is an object of stringstream that references the S string  
        // use while loop to check the getline() function condition  
        while (getline(X, T, '|')) 
        {  
            /* X represents to read the string from stringstream, T use for store the token string and, 
            '|' whitespace represents to split the string where whitespace is found. */  
            data.push_back(T);// print split string  
        } 
        if(data.size()==1){
            cout<<data[0]<<endl;
        }
        else{
            cout<<"Message From::  "<<data[0]<<endl;
            cout<<"Message     ::  "<<data[1]<<endl;
            cout<<"Time        ::  "<<data[2]<<endl;
        
        }
        cout<<'>';
    }
    // cout << "#" << i << " tag="<< message->getDeliveryTag() << " content-type:"<< message->getHeader("Content-type") ;
	// cout << " encoding:"<< message->getHeader("Content-encoding")<< " mode="<<message->getHeader("Delivery-mode")<<endl;
	return 0;
};

int main()
{
   
    int loggedIn=0;// global for login session
    cout<<"*****************************************************************"<<endl;
    cout<<"*                                                               *"<<endl;
    cout<<"*             ===WELCOME TO OUR CHAT APPLICATION===             *"<<endl;
    cout<<"*                                                               *"<<endl;
    cout<<"*****************************************************************"<<endl;
    Client *cl =Client::getInstance();  
    int sockStatus; 
    availableOptions:
    showOption();//options
    char option;
    cout<<"Enter Your Choice :: ";
    cin>>option;
    int invalidOptionFlag=0;
    switch(option){
        case '1':
            cout<<"You Selected Option 1"<<endl; 
            sockStatus=cl->createConnection();
            if(sockStatus>0)
            {
                cout<<"connection established"<<endl;
                user=showSignUpWindowandCollectData(); // getting user data for registration
                string userString =user.toString();
                string SignUp="SIGNUP";
                string data =SignUp+"|"+userString;
                char *dataptr = &data[0];
                char recBuf[1000]={0,};
                cout<<"Sending Data....."<<endl;
                if(send(cl->getSock(),dataptr,data.length(),0)>0){
                    if(recv(cl->getSock(),(char*)&recBuf,sizeof(recBuf),0)>0){
                        if(strcmp(recBuf,"SUCCESS")==0)
                        {
                        cout<<"Registration Successful"<<endl;
                        goto availableOptions;
                        }
                        else
                        {
                            cout<<"Registration Failed"<<endl;
                            int rFail=recv(cl->getSock(),(char*)recBuf,sizeof(recBuf),0);
                            if(rFail>0){
                                cout<<recBuf<<endl;
                            }
                            else{
                                cout<<"some unexpected situation occured"<<endl;
                                goto availableOptions;
                            }
                            
                        }
                    }
                    
                }else{
                    cout<<"Registration Request could not be sent"<<endl;
                }

                

            }
            else
            {
                cout<<"unable to Establish Connection"<<endl;
            }
            
            break;
        case '2':
            cout<<"You Selected Option 2"<<endl; 
            sockStatus=cl->createConnection();
            if(sockStatus>0)
            {
                loginUser = showSignInWindowAndCollectData();
                string userString=loginUser.toString();
                string Login="LOGIN";
                string data=Login+"|"+userString;
                char *sendBuf = &data[0];
                char recBuf[1025]={0,};
                cout<<"sending data to login"<<endl;
                if(send(cl->getSock(),sendBuf,data.length(),0)>0)
                {
                    if(recv(cl->getSock(),(char*)&recBuf,sizeof(recBuf),0)>0)
                    {
                        if(strcmp(recBuf,"SUCCESS")==0)
                        {
                            cout<<"Login Successful"<<endl;
                            loggedIn=1;
                            goto availableOptions;
                        }
                        else if (strcmp(recBuf,"NOT_REGISTERED")==0){
                            cout<<"User Not Registered"<<endl;
                            cout<<"Register first"<<endl;
                            goto availableOptions;
                        }       
                        else
                        {
                            cout<<"Login Failed"<<endl;
                            cout<<recBuf<<endl;
                            int loginfailReceive=recv(cl->getSock(),(char*)recBuf,sizeof(recBuf),0);
                                if(loginfailReceive>0){
                                    cout<<recBuf<<endl;
                                }
                                else{
                                    cout<<"something unexpected or something like forceful shutdown at server end occured"<<endl;
                                    goto availableOptions;
                                }
                            
                        }
                    }
                    else
                    {
                        cout<<"some unexpected occured not able to receive anything for success of login"<<endl;
                        goto availableOptions;
                    }
                }
                else
                {
                    cout<<"send not successful"<<endl; 
                }
            }
            else
            {
                cout<<"Not Able to connect to Server"<<endl;
            }
            break;
        case '3':
            if(loggedIn)
            {
                string chat="CHAT";
                sockStatus=cl->createConnection();
                if(sockStatus>0)
                {
                    char recBuf[1024]={0,};
                    thread receive([cl]()
                    {
                        try{
                            string queue=loginUser.getEmail();
                            AMQPQueue * qu = (cl->getAMQP())->createQueue(queue);
                            cout<<"waiting of message"<<endl;
                            qu->Declare(queue,AMQP_DURABLE);
                            qu->Bind( "ChatApp", queue);
                            qu->setConsumerTag(queue);
                            qu->addEvent(AMQP_MESSAGE, onMessage);
                            qu->addEvent(AMQP_CANCEL, onCancel );

                            qu->Consume(AMQP_NOACK);//
                        }
                        catch (AMQPException e) {
                            std::cout << e.getMessage() << std::endl;
                        }                          
                    }
                    );
                    if(receive.joinable()){
                        receive.detach();
                    }
                    // this while loop is for sending message;                
                    while(1)
                    {
                        startChat:
                        char msg[1025];
                        cout << " You can enter Exit here if you want to exit chat screen else you can enter your destination\n";
                        string dest;
                        cout<<"Enter Your Destination:: ";  
                        cin>>dest;
                        if(strcmp(dest.c_str(),"exit")==0)
                        {
                            cout<<"you opted to exit chat"<<endl;
                            // string logout="LOGOUT"; /* here we are logging out just to say user is not available beacuse there is
                            // no feature yet to receive message from another user until we are in chat box*/
                            // string msgString=logout+"|"+user.getEmail();
                            // strcpy(msg,msgString.c_str());
                            // send(cl->getSock(), (char*)&msg, strlen(msg), 0);
                            goto availableOptions;
                                                    
                        }
                        else if(dest==loginUser.getEmail()){
                            cout<<"DESTINATION AND SOURCE CAN NOT BE SAME"<<endl;
                            goto startChat;
                        }
                        cout<<"Enter Your Message:: "<<endl;
                        string messageBody;
                        // getline(cin,messageBody);
                        cin.ignore();
                        getline(cin,messageBody);
                        // get current time
                        time_t rawtime;
                        struct tm * timeinfo;
                        char buffer[200];
                        time (&rawtime);
                        timeinfo = localtime(&rawtime);
                        strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
                        string timeStamp(buffer);
                        string from =loginUser.getEmail();
                        cout<<"From:: "<<from<<endl;
                        string data="";
                        data = chat+"|"+from+"|"+dest+"|"+messageBody+"|"+timeStamp;
                        memset(&msg, 0, sizeof(msg));//clear the buffer
                        strcpy(msg, data.c_str());
                        send(cl->getSock(), (char*)&msg, strlen(msg), 0);
                    }
                    
                }
                else
                {
                    cout<<"Not Able to connect to Server"<<endl;
                    goto availableOptions;
                }
            }
            else
            {
                cout<<"Go & Login First"<<endl;
                goto availableOptions;
            }
            break;
        case '4':
            if(1){
                cout<<"You selected option 4"<<endl;
                cout<<"You Selected to Exit........."<<endl;
                string logout="LOGOUT";
                string msgString=logout+"|"+user.getEmail();
                const char* msg=msgString.c_str();
                send(cl->getSock(), (char*)msg, strlen(msg), 0);
            }
            

            break;
        default:
            cout<<"You Selected Invalid Option"<<endl;
            invalidOptionFlag=1;
            break;
    }

    if(invalidOptionFlag==1)
    {
        goto availableOptions;
    }
    
    return 0; 
}



