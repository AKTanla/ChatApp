#include <Poco/AutoPtr.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <iostream>

using namespace std;
using Poco::AutoPtr;
using Poco::Util::IniFileConfiguration;

int main(){
    AutoPtr<IniFileConfiguration> pConf(new IniFileConfiguration("config.ini"));
    std::string host = pConf->getString("Database.host");
    int port = pConf->getInt("Database.port");
    cout<<host<<endl;
    // value = pConf->getInt("myapplication.SomeValue");
    cout<<port<<endl;
    std::string password = pConf->getString("Database.password");
    cout<<password<<endl;

    return 0;
}
