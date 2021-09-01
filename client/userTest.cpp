#include <iostream>
#include <string>
#include "user.h"

using namespace std;

int main(){
    RegisterUser user("Aman","Kumar","AKumar","akumar@gmail.com","Aman@123");
    cout<<user.getEmail()<<endl;
    return 0;
}

