#include "user.h"

User::User(string accountName, string passwd)
	:accountName_(""), 
	passwd_(""), 
	id_(0),
	status_(Offline)
{
}

void User::signUp()
{
}

bool User::sendMessageToUser(User to, string content) { return false; }

bool User::sendMessageToGroupChat(int id, string content) {return false; }

void User::setStatus(UserStatus status) { status_ = status; }
UserStatus User::getStatus() const { return status_; }

void User::addConversation(PrivateChat conversation) {}

void User::addConversation(GroupChat conversation) {}
void User::createGroup(){}
int  User::getId() { return id_; }
std::string User::getAccountName() { return accountName_; }

