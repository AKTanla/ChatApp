#include "AMQPcpp.h"
#include <string.h>
#include <cstring>
#include <mysql/mysql.h>
#include <string>
#include <sstream>

using namespace std;
char *host;
char *user;
char *pass;
char *dbName;
static char * unix_socket;
static unsigned int flag;
static unsigned int sqlport;
MYSQL *conn;



int i=0;

int onCancel(AMQPMessage * message ) {
	cout << "cancel tag="<< message->getDeliveryTag() << endl;
	return 0;
}

int  onMessage( AMQPMessage * message  ) {
    uint32_t j = 0;
    char * data = message->getMessage(&j);
    string tempData=string(data);
    // AMQPQueue * qu = message->getQueue();
    // qu->Ack(message->getDeliveryTag());
    if (data){
        string query;
        int queryStatus;
        vector<string> signUpdata;
        string T;  // declare string variables  
        stringstream X(tempData); // X is an object of stringstream that references the S string  

        // use while loop to check the getline() function condition  
        while (getline(X, T, '|')) {  
            /* X represents to read the string from stringstream, T use for store the token string and, 
            '|' pipe represents to split the string where pipe is found. */  
            signUpdata.push_back(T);// store split string  
        }  
        getData:
        query="SELECT user_id FROM user WHERE user_email='"+signUpdata[0]+"'";
        queryStatus=mysql_query(conn,query.c_str());
        if(!queryStatus){
            MYSQL_RES *res;
            res =mysql_store_result(conn);
            MYSQL_ROW row;
            int numRow =mysql_num_rows(res);
            row = mysql_fetch_row(res); 
            if(numRow>0){
                string user_id=row[0];
                mysql_free_result(res);
                savedata:
                query="INSERT INTO signUpReport(signUp_id,signUp_time) VALUES ("+user_id+",'"+signUpdata[1]+"')";
                queryStatus=mysql_query(conn,query.c_str());
                cout<<queryStatus<<endl;
                if(!queryStatus){
                    cout<<"data saved to database"<<endl;
                    AMQPQueue * qu = message->getQueue();
                    qu->Ack(message->getDeliveryTag());
                }
                else
                {
                    cout<<"try again";
                    cout<<mysql_error(conn)<<endl;
                    goto savedata;
                }
            }
            else{
                cout<<"some Error occured"<<endl;
            }
        }
        else{
            cout<<"some error occured try again"<<endl;
            goto getData;
        }
        
    }
    return 0;
};


int main () {

    // connect to my sql first

    conn = mysql_init(NULL);
    host="localhost", user="root", pass="Welcome#$123", dbName="ChatSocket";
    sqlport=3306;
    unix_socket=NULL;
    flag=0;
    if(!mysql_real_connect(conn,host,user,pass,dbName,sqlport,unix_socket,flag))
	{
		fprintf(stderr,"Error: %s [%d]",mysql_error(conn),mysql_errno(conn));
		exit(1);
	}

    try {

		AMQP amqp("AK.Tanla:Welcome#$123@localhost:5672//");

		AMQPQueue * qu = amqp.createQueue("signUpReport");

		qu->Declare("signUpReport",AMQP_DURABLE);
		qu->Bind( "ChatApp", "signUpReport");

		qu->setConsumerTag("signUpReport");
		qu->addEvent(AMQP_MESSAGE, onMessage );
		qu->addEvent(AMQP_CANCEL, onCancel );

		qu->Consume();//


	} catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}

	return 0;

}

