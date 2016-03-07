#include "user.h"

User::User()
	:id_(0),
	accountName_(""),
	passwd_(""),
	status_(Offline)
{
}

User::User(std::string accountName, std::string passwd)
	:id_(0),
	accountName_(accountName), 
	passwd_(passwd), 
	status_(Offline)
{
}

SignUp User::signUp()
{
  std::string name;
	std::string passwd;
	std::cout << "Sign Up: \n";
	std::cout << "user name:\n";
	getline(std::cin, name);
	std::cout << " password:\n";
	getline(std::cin, passwd);
	SignUp signUp;
	signUp.set_name(name);
	signUp.set_passwd(passwd);
	accountName_ = name;
	passwd_ = passwd;
	return signUp;
}

PMessage User::sendMessageToUser(int64_t uid, std::string content) 
{ 
	PMessage message;
	message.set_uid(uid);
	message.set_content(content);
	return message; 
}


GMessage User::sendMessageToGroupChat(int64_t gid, std::string content)
{
	GMessage message;
  message.set_gid(gid);
	message.set_content(content);
	return  message; 
}

void User::setId(int64_t id) { id_ = id; }



UserStatus User::getStatus() const { return status_; }

void User::addConversation(PrivateChat conversation) {}

void User::addConversation(GroupChat conversation) {}
void User::createGroup(){}

int64_t  User::getId() { return id_; }

std::string User::getAccountName( ) { return accountName_; }

std::string User::getPassword() { return passwd_; }


