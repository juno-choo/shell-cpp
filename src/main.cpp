#include <iostream>
#include <string>
#include <vector>
#include <sstream> // For stringstream (parsing PATH)
#include <unistd.h> // For access(), X_OK, getcwd()
#include <sys/wait.h> // For waitpid
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
  // Gets the path
  string pathEnv = getenv("PATH");

  // Splits
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

vector<string> splitLine(string line) {
  vector<string> args;
  string word;
  stringstream ss(line);

  // Keep reading while there is a word
  while (ss >> word) {
    args.push_back(word);
  }
  return args;
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
        string path = getPath(line);
        if (!path.empty()) {
          cout << line << " is " << path << endl;
        }
        else {
          cout << line << ": not found" << endl;
        }
      }
    }
    else if (command == "pwd") {
      char buffer[1024];
      cout << getcwd(buffer, sizeof(buffer)) << endl;
    }
    else if (command == "cd") {
      int res = chdir(line.c_str());

      if (res == -1) {
        cout << "cd: " << line << ": No such file or directory" << endl;
      }
    }
    else {
      string path = getPath(command);

      if (path.empty()) {
        cout << command << ": command not found " << endl; 
      } 
      else {
        // Prepare arguments
        vector<string> argParts = splitLine(line);
        // Create vector of C-string for execv
        vector<char*> args;
        args.push_back(const_cast<char*>(command.c_str()));

        for (auto& part : argParts) {
          args.push_back(const_cast<char*>(part.c_str()));
        }
        args.push_back(NULL);

        // Fork and execute
        pid_t pid = fork();
        // CHILD PROCESS
        if (pid == 0) {
          // execv replaces the shell's process with the intened program's
          execv(path.c_str(), args.data());

          perror("execv failed.");
          exit(1);
        }
        // PARENT PROCESS
        else if (pid > 0) {
          int status;
          // Pauses until child is done
          waitpid(pid, &status, 0);
        }
      }
    } 
  }   
  return 0;
}
