#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  bool running = true;

  while (running) {
    cout << "$ ";

    string command;
    getline(cin, command);
    cout << command << ": command not found " << endl;

    if (command == "exit") {
      running = false;
    }
  } 

  return 0;
}
