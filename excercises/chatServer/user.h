#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include "Conversation.h"
#include "msg.pb.h"
using namespace im;

#include <boost/unordered_map.hpp>


enum UserStatus
{
	Offline, Away, Idle, Available, Busy
};

class PrivateChat;
class GroupChat;
class User : boost::noncopyable
{
	friend class PrivateChat;
	friend class GroupChat;
public:
	User();
  User(std::string accountName, std::string passwd);

	SignUp signUp();
  PMessage sendMessageToUser(int64_t uid, std::string content);
  GMessage sendMessageToGroupChat(int64_t gid, std::string content);

	void setId(int64_t id);
  void setStatus(UserStatus status) { status_ = status; }
	void setAccountName(std::string name) { accountName_ = name; }
	void setPassword(std::string passwd) { passwd_ = passwd; }
  UserStatus getStatus() const;
//  bool addContact(User user);
//  void receivedAddRequest(AddRequest req);
//  void sentAddRequest(AddRequest req);
//  void removeAddRequest(AddRequest req);
//  void requestAddUser(std::string accountName);
  void addConversation(PrivateChat conversation);
  void addConversation(GroupChat conversation);
	void createGroup();
  int64_t getId();
  std::string getAccountName();
	std::string getPassword();

private:
  int id_;
  std::string accountName_;
	std::string passwd_;
  UserStatus status_;
  
  //others'id -> chat
  boost::unordered_map<int, PrivateChat> privateChats_;

  //groupId -> group
	boost::unordered_map<int, GroupChat> groupChats_;

  //map others'id to addRequest
//  boost::unordered_map<int, AddRequest> receivedAddRequests_;

  //map others'id to AddRequest
//  boost::unordered_map<int, AddRequest> sentAddRequests_;

  //map users'id to User
  //boost::unordered_map<int, User> contacts_;

};

#endif
