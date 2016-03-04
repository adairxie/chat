#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <list>
#include <vector>
#include <string>
#include <map>
#include "../../Timestamp.h"

#include "user.h"
#include <boost/noncopyable.hpp>

class User;

class Conversation : boost::noncopyable
{
public:
 Conversation(std::string name);
 std::string getMessages(Timestamp);
 void addMessage(std::string message);
 int getId();

protected:
  int id_;
	std::string name_; 
  std::vector<User> participants_;
	std::map<Timestamp, std::string> messages_;
};

class GroupChat : public Conversation 
{
public:
	GroupChat(std::string name):Conversation(name) { }
  void removeParticipant(User user);
  void addParticipant(User user);
};

class PrivateChat : public Conversation 
{
public:  
  PrivateChat(User user1, User user2);
  User getOtherParticipant(User participant);
};


#endif // CONVERSATION_H
