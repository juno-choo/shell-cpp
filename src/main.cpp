#include <iostream>
#include <string>
#include <vector>
using namespace std;

const vector<string> BUILTINS = {"echo", "exit", "cd", "ls", "pwd"};

bool isBuiltin(const string& cmd) {
  for (const auto& builtin : BUILTINS) {
    if (cmd == builtin) {
      return true;
    }
  }
  return false;
}
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
    else if (command == "type") {
      if (isBuiltin(line)) {
        cout << line << " is a shell builtin" << endl;
      } 
      else {
        cout << line << " not found" << endl;
      }
    }
    else {
      cout << command << ": command not found " << endl;  
    } 
  }   
  return 0;
}
