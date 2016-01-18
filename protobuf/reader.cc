#include "msg.pb.h"
#include <fstream>
#include <iostream>
using namespace std;

void ListMsg(const lm::helloworld& msg)
{
  cout << msg.id() << endl;
  cout << msg.str() << endl;
}

int main(int argc, char* argv[])
{
  lm::helloworld msg1;
  {
    fstream input("./log", ios::in | ios::binary);
    if (!msg1.ParseFromIstream(&input)) {
      cerr << "Failed tot parse address book." << endl;
      return -1;
    }
  }

  ListMsg(msg1);
}
