#include "client.h"
#include <iostream>
#include <ostream>
#include <string>
#include <cstring>
#include <string.h>
#include <stdio.h>
#include "user.h"
#include <boost/regex.hpp>
#include <curses.h>
#include<limits>
#include<ios>

using namespace std;
using namespace boost;

void saveToLocalStorage(){
    
}

void showOption(){
    cout<<"*****************************************************************"<<endl;
    cout<<"*            HERE ARE YOUR OPTIONS                              *"<<endl;
    cout<<"*            1. TO SIGN UP                                      *"<<endl;
    cout<<"*            2. TO SIGN IN                                      *"<<endl;
    cout<<"*            3. TO CONTINUE WITHOUT SIGN IN                     *"<<endl;
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

    cout<<"returning"<<endl;
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

int main()
{
    cout<<"*****************************************************************"<<endl;
    cout<<"*                                                               *"<<endl;
    cout<<"*             ===WELCOME TO OUR CHAT APPLICATION===             *"<<endl;
    cout<<"*                                                               *"<<endl;
    cout<<"*****************************************************************"<<endl;
    Client *cl =Client::getInstance();  
    int sockStatus; 
    sockStatus=cl->createConnection();
    availableOptions:
    showOption();//options
    char option;
    cout<<"Enter Your Choice :: ";
    cin>>option;
    int invalidOptionFlag=0;
        switch(option)
        {
            case '1':
                cout<<"You Selected Option 1"<<endl; 
                if(sockStatus>0)
                {
                    RegisterUser user;
                    cout<<"connection established"<<endl;
                    user=showSignUpWindowandCollectData(); // getting user data for registration
                    string userString =user.toString();
                    string SignUp="SIGNUP";
                    string data =SignUp+"|"+userString;
                    char *dataptr = &data[0];
                    char recBuf[1000];
                    cout<<"Sending Data....."<<endl;
                    if(send(cl->getSock(),dataptr,data.length(),0)>0){
                        if(recv(cl->getSock(),(char*)&recBuf,sizeof(recBuf),0)>0){
                            if(strcmp(recBuf,"SUCCESS")==0){
                            cout<<"Registration Successful"<<endl;



                        }else{
                            cout<<"Registration Failed"<<endl;
                            recv(cl->getSock(),(char*)recBuf,sizeof(recBuf),0);
                            cout<<recBuf<<endl;
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
                if(sockStatus>0)
                {
                    LoginUser loginUser = showSignInWindowAndCollectData();
                    string userString=loginUser.toString();
                    string Login="LOGIN";
                    string data=Login+"|"+userString;
                    char *sendBuf = &data[0];
                    char recBuf[1025]={0,};
                    cout<<"sending data to login"<<endl;
                    if(send(cl->getSock(),sendBuf,data.length(),0)>0){
                        if(recv(cl->getSock(),(char*)&recBuf,sizeof(recBuf),0)>0){
                                if(strcmp(recBuf,"SUCCESS")==0){
                                cout<<"Login Successful"<<endl;

                            }else{
                                cout<<"Login Failed"<<endl;
                                recv(cl->getSock(),(char*)recBuf,sizeof(recBuf),0);
                                cout<<recBuf<<endl;
                            }
                        }
                    }
                }
                else{
                    cout<<"Not Able to connect to Server"<<endl;
                }
                break;
            case '3':
                cout<<"You Selected option 3"<<endl; 
                if(sockStatus>0)
                {
                    LoginUser loginUser = showSignInWindowAndCollectData();
                    string userString=loginUser.toString();
                    string Login="LOGIN";
                    string data=Login+"|"+userString;
                    char *sendBuf = &data[0];
                    char recBuf[1025]={0,};
                    cout<<"sending data to login"<<endl;
                    if(send(cl->getSock(),sendBuf,data.length(),0)>0){
                        if(recv(cl->getSock(),(char*)&recBuf,sizeof(recBuf),0)>0){
                                if(strcmp(recBuf,"SUCCESS")==0){
                                cout<<"Login Successful"<<endl;

                            }else{
                                cout<<"Login Failed"<<endl;
                                recv(cl->getSock(),(char*)recBuf,sizeof(recBuf),0);
                                cout<<recBuf<<endl;
                            }
                        }
                    }
                }
                else{
                    cout<<"Not Able to connect to Server"<<endl;
                }
                break;
            case '4':
                cout<<"You selected option 4"<<endl;
                cout<<"You Selected to Exit........."<<endl;
                return 0;
                break;
            default:
                cout<<"You Selected Invalid Option"<<endl;
                invalidOptionFlag=1;
                break;
        }
    if(invalidOptionFlag==1){
        goto availableOptions;
    }
    
   return 0; 
}