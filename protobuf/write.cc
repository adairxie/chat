#include "msg.pb.h"
#include <fstream>
#include <iostream>
using namespace std;

int main(void)
{
  lm::helloworld msg1;
  msg1.set_id(101);
  msg1.set_str("hello");
  fstream out("./log", ios::out | ios::trunc | ios::binary);

  if (!msg1.SerializeToOstream(&out)) {
    cerr << "Failed to write msg." << endl;
    return -1;
  }
  return 0;
}
