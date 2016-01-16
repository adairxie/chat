#ifndef USER_H
#define USER_H

#include <boost/unordered_map.hpp>

class User : boost::noncopyable
{
public:
  User(int id, string accountName, string);
  bool sendMessageToUser(User to, string content);
  bool sendMessageToGroupChat(int id, string content);
  void setStatus(UserStatus status);
  UserStatus getStatus();
  bool addContact(User user);
  void receivedAddRequest(AddRequest req);
  void sentAddRequest(AddRequest req);
  void removeAddRequest(AddRequest req);
  void requestAddUser(string accountName);
  void addConversation(PrivateChat conversation);
  void addConversation(GroupChat conversation);
  int getId();
  string getAccountName();
  string getFullName();

private:
  int id_;
  UserStatus status_;
  
  //others'id -> chat
  boost::unordered_map<int, PrivateChat> privateChats_;

  //groupId -> group
  vector<GroupChat> groupChats_;

  //map others'id to addRequest
  boost::unordered_map<int, AddRequest> receivedAddRequests_;

  //map others'id to AddRequest
  boost::unordered_map<int, AddRequest> sentAddRequests_;

  //map users'id to User
  boost::unordered_map<int, User> contacts_;

  string accountName_;
  string fullName_;

};

#endif
