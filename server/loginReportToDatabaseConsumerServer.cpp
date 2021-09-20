#include "AMQPcpp.h"
#include <string.h>
#include <cstring>
#include <mysql/mysql.h>

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
    int queryStatus;
    string query= string(data);
    cout<<query<<endl;
    // AMQPQueue * qu = message->getQueue();
    // qu->Ack(message->getDeliveryTag());
    if (data){
        savedata:
        queryStatus=mysql_query(conn,query.c_str());
        cout<<queryStatus<<endl;
        if(!queryStatus){
            cout<<"data saved to database"<<endl;
            AMQPQueue * qu = message->getQueue();
            qu->Ack(message->getDeliveryTag());
        }else{
            cout<<"try again";
            cout<<mysql_error(conn)<<endl;
            goto savedata;
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

		AMQPQueue * qu = amqp.createQueue("loginReport");

		qu->Declare("loginReport",AMQP_DURABLE);
		qu->Bind( "ChatApp", "loginReport");

		qu->setConsumerTag("loginReport");
		qu->addEvent(AMQP_MESSAGE, onMessage );
		qu->addEvent(AMQP_CANCEL, onCancel );

		qu->Consume();//


	} catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}

	return 0;

}

