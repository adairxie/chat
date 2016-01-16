#ifndef USERMANAGER_H
#define USERMANAGER_H
#include "user.h"

#include <boost/unordered_map.hpp>
#include <boost/uncopyable.hpp>

using std::string;

class UserManger : boost::noncopyable
{
private:
  static UserManger instance_;
  //userID -> User
  boost::unordered_map<int, User> usersById;

  //accountName -> User
  boost::unordered_map<string, User> usersByAccountName;

  //userId -> onlineUser
  boost::unordered_map<int, User> onlineUsers;


public:
  static UserManager getInstance() {
    if (!instance_) instance_ = new UserManager();
    return instance_;
  }

  void addUser(User fromUser, string toAccountName);
  void approveAddRequest(AddRequest req);
  void rejectAddRequest(AddRequest req);
  void userSignedOn(string accountName);
  void userSignedOff(string accountName);

};

#endif //USERMANAGER_H
