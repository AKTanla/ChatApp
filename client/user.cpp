#include "user.h"

// template<class Archive>
// void RegisterUser::serialize(Archive& archive, const unsigned int version){
//     archive & firstName;
//     archive & lastName;
//     archive & userName;
//     archive & email;
//     archive & password;
// }


RegisterUser::RegisterUser()
{
    firstName="";
    lastName="";
    userName="";
    email="";
    password="";

}

RegisterUser::RegisterUser(string firstName, string lastName, string userName, string email, string password){
            this->firstName=firstName;
            this->lastName=lastName;
            this->userName=userName;
            this->email=email;
            this->password=password;
        }

// getters here
string RegisterUser::getFirstName() const {
            return this->firstName;
        }

string RegisterUser::getLastName() const {
            return this->lastName;
        }

string RegisterUser::getUserName() const {
            return this->userName;
        }

string RegisterUser::getEmail() const {
            return this->email;
        }

string RegisterUser::getPassword() const {
            return this->password;
        }

    // setters here
void RegisterUser::setFirstName(string firstName){
            this->firstName=firstName;
        }

void RegisterUser::setLastName(string lastName){
            this->lastName=lastName;
        }

void RegisterUser::setUserName(string userName){
            this->userName=userName;
        }

void RegisterUser::setEmail(string email){
            this->email=email;
        }

void RegisterUser::setPassword(string password){
            this->password=password;
        }

// show Data
void RegisterUser::showRegisterUser(){
        cout<<"First Name :: "<<this->getFirstName()<<endl;
        cout<<"Last Name :: "<<this->getLastName()<<endl;
        cout<<"User Name :: "<<this->getUserName()<<endl;
        cout<<"User Email :: "<<this->getEmail()<<endl;
        cout<<"User Password :: "<<this->getPassword()<<endl;
    }

// to string object

string RegisterUser::toString(){
        string result="";
        result=this->firstName+"|"+this->lastName+"|"+this->userName+"|"+this->email+"|"+this->password;
        return result;
        
    }


// save function

// void RegisterUser::save(ostream &oss)
// {
//     boost::archive::text_oarchive oa(oss);
//     oa & *(this);
// }

// void RegisterUser::load(string str_data)
// {
//     std::istringstream iss(str_data);
//     boost::archive::text_iarchive ia(iss);
//     ia & *(this);
// }


// constructors
LoginUser::LoginUser(){
    userEmail="";
    password="";
}
LoginUser::LoginUser(string userEmail, string password){
    this->userEmail=userEmail;
    this->password=password;
}
LoginUser::~LoginUser(){}

// template<class Archive>
// void LoginUser::serialize(Archive& archive, const unsigned int){
//     archive & userNameOrEmail;
//     archive & password;
// }

// getters Here

string LoginUser::getEmail() const {
    return this->userEmail;
}
string LoginUser::getPassword() const{
    return this->password;
}

// setters here
void LoginUser::setEmail(string Email){
    this->userEmail=Email;
}
void LoginUser::setPassword(string password){
    this->password=password;
}

// to string object

string LoginUser::toString(){
        string result="";
        result=this->userEmail+"|"+this->password;
        return result;
        
    }

// // save to stream function
// // to convert object to stream 
// void LoginUser::save(ostream &oss){
//     boost::archive::text_oarchive oa(oss);
//     oa & *(this);
// }
// // to get object back
// void LoginUser::load(string str_data){
//     std::istringstream iss(str_data);
//     boost::archive::text_iarchive ia(iss);
//     ia & *(this);
// }


BOOST_CLASS_VERSION(RegisterUser, 1)
BOOST_CLASS_VERSION(LoginUser, 1)
