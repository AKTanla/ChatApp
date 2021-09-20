#include <mysql/mysql.h>

using namespace std;


class SQLServer{
    char *host;
    char *user;
    char *pass;
    char *dbName;
    static char * unix_socket;
    static unsigned int flag;
    static unsigned int port;

    public:
        SQLServer();
        SQLServer(char *host, char *user, char *password, char *dbName){
            this->host=host;
            this->user=user;
            this->password=password;
            this->dbName=dbName;
        }

        

}