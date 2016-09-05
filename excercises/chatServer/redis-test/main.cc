#include <iostream>
#include <fstream>
#include "../msg.pb.h"
#include <hiredis/hiredis.h>

using namespace im;

int main(void)
{

	PMessage pmsg;
	pmsg.set_uid(10);
	pmsg.set_peerid(9);
	pmsg.set_content("hello");
	
	const int byteSize = pmsg.ByteSize();
	std::cout << "byteSize = " << byteSize << std::endl;
	char buf[byteSize];
	bzero(buf, byteSize);
	pmsg.SerializeToArray(buf, byteSize);


	redisContext *c;
	redisReply *reply;

	struct timeval timeout = {1,500000};
	c = redisConnectWithTimeout((char*) "127.0.0.1", 6379, timeout);
	if (c->err) {
		printf("Connection error: %s\n", c->errstr);
		exit(1);
	}

/*	reply = (redisReply*) redisCommand(c, "RPUSH %ld %b", pmsg.peerid(),buf, byteSize);
	reply = (redisReply*) redisCommand(c, "RPUSH %ld %b", pmsg.peerid(),buf, byteSize);
	reply = (redisReply*) redisCommand(c, "RPUSH %ld %b", pmsg.peerid(),buf, byteSize);
	freeReplyObject(reply);
*/  

	reply = (redisReply*) redisCommand(c, "LRANGE %d %d %d", pmsg.peerid(), -1, -1);
	
	PMessage pmsg2;
	pmsg2.ParseFromArray(reply->element[0]->str, reply->element[0]->len);

	std::cout <<"pmsg2.uid: " << pmsg2.uid() << " pmsg2.peerid: " << pmsg2.peerid();
	return 0;
}
