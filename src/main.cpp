#include <iostream>
#include <string>
#include <vector>
#include <sstream> // For stringstream (parsing PATH)
#include <unistd.h> // For access() and X_OK
using namespace std;

const vector<string> BUILTINS = {"echo", "exit", "cd", "pwd", "type"};

bool isBuiltin(const string& cmd) {
  for (const auto& builtin : BUILTINS) {
    if (cmd == builtin) {
      return true;
    }
  }
  return false;
}

string getPath(string cmd) {
  // Gets the path of 
  string pathEnv = getenv("PATH");

  stringstream ss(pathEnv);
  string pathSegment;

  while (getline(ss, pathSegment, ':')) {
    string absPath = pathSegment + '/' + cmd;

    if (access(absPath.c_str(), X_OK) == 0) {
      return absPath;
    }
  }
  return "";
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
        string path = getPath(command);
        if (!path.empty()) {
          cout << line << " is " << path << endl;
        }
        else {
          cout << line << ": not found" << endl;
        }
      }
    }
    else {
      cout << command << ": command not found " << endl;  
    } 
  }   
  return 0;
}
