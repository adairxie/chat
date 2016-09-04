#include <iostream>
#include <fstream>
#include "seri.pb.h"
#include <hiredis/hiredis.h>



int main(void)
{
	seri::User u;
	u.set_id(1);
	u.set_username("Jack");
	u.set_password("123456");
	u.set_email("2312312@qq.com");

	seri::Person* _person1 = u.add_person();
	_person1->set_id(1);
	_person1->set_name("P1");

	seri::PhoneNumber* _phone1 = _person1->add_phone();
	_phone1->set_number("+1232131231sa");
	_phone1->set_type(seri::MOBILE);

  seri::PhoneNumber* _phone2 = _person1->add_phone();
	_phone2->set_number("02823334717");
	_phone2->set_type(seri::WORK);

	seri::Person* _person2 = u.add_person();
	_person2->set_id(2);
	_person2->set_name("P2");

	seri::PhoneNumber* _phone3 = _person2->add_phone();
	_phone3->set_number("028123193919");
	_phone3->set_type(seri::WORK);

	const int byteSize = u.ByteSize();
	std::cout << "byteSize = " << byteSize << std::endl;
	char buf[byteSize];
	bzero(buf, byteSize);
	u.SerializeToArray(buf, byteSize);


	redisContext *c;
	redisReply *reply;

	struct timeval timeout = {1,500000};
	c = redisConnectWithTimeout((char*) "127.0.0.1", 6379, timeout);
	if (c->err) {
		printf("Connection error: %s\n", c->errstr);
		exit(1);
	}

	/*reply = (redisReply*) redisCommand(c, "SET %b %b", u.username().c_str(), (int)u.username().length(), buf, byteSize);
	printf("SET (binary API): %s\n", reply->str);
	freeReplyObject(reply);
  */

	reply = (redisReply*) redisCommand(c, "Get Jack");
	std::cout << "reply->len" << reply->len << "\nreply->str : \n" << reply->str << std::endl;
	
	std::cout << "----------------------------------------"<<std::endl;

	seri::User u2;
	u2.ParseFromArray(reply->str, reply->len);

	std::cout << u2.id() << std::endl;
	std::cout << u2.username() <<std::endl;
	std::cout << u2.password() << std::endl;
	std::cout << u2.email() << std::endl;

	std::cout << "----------------------------------------" << std::endl;
	for (int i=0; i < u2.person_size(); i++)
	{
		seri::Person* p = u2.mutable_person(i);
		std::cout << p->id() << std::endl;
		std::cout << p->name()<< std::endl;
		for (int j=0; j < p->phone_size();j++) {
			seri::PhoneNumber* phone = p->mutable_phone(j);
			std::cout << phone->number() << std::endl;
		}
		std::cout << "------------------------------------" << std::endl;
	}
	return 0;
}
