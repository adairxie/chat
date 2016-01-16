#ifndef ADDREQUEST_H
#define ADDREQUEST_H

enum RequestStatus
{
  Unread, Read, Accepted, Rejected
};

class AddRequest
{
public:
  AddRequest(User from, User to, Date date);
  RequestStatus getStatus();
  User getFromUser();
  User getToUser();
  Date getDate();
private:
  User fromUser_;
  User toUser_;
  Date date_;
  RequestStatus status_;
};

#endif //ADDREQUEST_H
