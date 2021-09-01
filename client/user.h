#ifndef __USER_H
#define __USER_H 

#include <iostream>
#include <string>
#include <sstream> 

// boost library for binary archine serialization
#include<boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;

// class declaration for 
class RegisterUser{

    // friend class boost::serialization::access;

    string firstName;
    string lastName;
    string userName;
    string email;
    string password; 

    // template<class Archive>

    //    void serialize(Archive& , const unsigned int);

    public:
    // constuctors here
        RegisterUser();

        RegisterUser(string firstName, string lastName, string userName, string email, string password);

        ~RegisterUser(){

        }

    // getters here
        string getFirstName() const ;

        string getLastName() const ;

        string getUserName() const;

        string getEmail() const ;

        string getPassword() const ;

    // setters here
        void setFirstName(string firstName);

        void setLastName(string lastName);

        void setUserName(string userName);

        void setEmail(string email);

        void setPassword(string password);

    // show Data
    void showRegisterUser();

    // toString()

    string toString() ;
    // save to stream function
    // void save(ostream &oss);
    // void load(string str_data);
        
};

class LoginUser{

    // friend class boost::serialization::access;
    string userEmail;
    string password;

    

    // template<class Archive>

    //    void serialize(Archive& , const unsigned int);

    public:

    // constructors
    LoginUser();
    LoginUser(string userEmail, string password);
    ~LoginUser();

    // getters Here

    string getEmail() const;
    string getPassword() const;

    // setters here
    void setEmail(string Email);
    void setPassword(string password);


    string toString();

    // save to stream function
    // to convert object to stream 
    // void save(ostream &oss);
    // // to get object back
    // void load(string str_data);

};

#endif
