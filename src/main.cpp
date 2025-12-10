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
    string line;
    cin >> command;

    // Remove leading space before getline
    if (cin.peek() == ' ') {
        cin.get();
    }

    getline(cin, line); 

    if (command == "exit") {
      break;
    } 
    else if (command == "echo") {
      cout << line << endl;
    }
    else {
      cout << command << ": command not found " << endl;  
    } 
  }
  return 0;
}
