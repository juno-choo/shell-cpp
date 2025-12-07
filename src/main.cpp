#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  while (true) {
    cout << "$ ";

    string command;
    if (command == "exit") {
      break;
    } 
    else {
      getline(cin, command);
      cout << command << ": command not found " << endl;  
    } 
  }
  return 0;
}
