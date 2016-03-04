#include "Conversation.h"

Conversation::Conversation(std::string name="")
	:name_(name), id_(0)
{
}

void Conversation::addMessage(std::string message)
{
  messages_[Timestamp::now()]=message;	
}

std::string Conversation::getMessages(Timestamp) 
{
	return std::string("");
}

int Conversation::getId()
{
	return id_;
}

void GroupChat::removeParticipant(User user)
{

}

void GroupChat::addParticipant(User user)
{

}

PrivateChat::PrivateChat(User user1, User user2) { }

User PrivateChat::getOtherParticipant(User participant)
{

}


