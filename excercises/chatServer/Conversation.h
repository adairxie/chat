#ifndef CONVERSATION_H
#define CONVERSATION_H



class Message
{
public:
  Message(string content, Data date);
  string getContent();
  Date getDate();

private:
  string content_;
  Date date;
};


class Conversation : boost::noncopyable
{
public:
 list<Message> getMessages();
 bool addMessage(Message m);
 int getId();

protected:
  vector<User> participants_;
  int id_;
  list<Message> messages_;
};

class GroupChat : public Conversation 
{
public:
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
