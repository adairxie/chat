#ifndef USERSTATUS_H
#define USERSTATUS_H


enum UserStatusType
{
  Offline, Away, Idle, Available, Busy
};

class UserStatus
{
public:
  UserStatus getStaus();
  User getFromUser();
  User getToUser();
  Date getDate();;
private:
  string message_;
  UserStatusType type_;
};

#endif //USERSTATUS_H
