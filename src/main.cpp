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
  string current;
  char quoteChar = '\0';  // '\0' means not in quotes

  for (int i = 0; i < line.length(); ++i) {
    char c = line[i];
    
    if ((c == '\'' || c == '\"') && quoteChar == '\0') {
        // Starting a quoted section
        quoteChar = c;
        continue;
    }
    else if (c == '\\' && quoteChar == '\0') {
      if (i + 1 < line.length()) {
        current += line[i+1];
        i += 1; 
      }
      continue;
    }
    else if (c == '\\' && quoteChar == '"') {
      // Peek ahead, check if special, decide what to add
      if (i + 1 < line.length()) {
        if (line[i+1] == '"' || line[i+1] == '\\') {
          current += line[i+1];
          i += 1; 
          continue;
        }
      }
      current += c;
    }
    else if (c == quoteChar) {
        // Ending the quoted section (same quote type)
        quoteChar = '\0';
        continue;
    }
    else if (quoteChar == '\0' && c == ' ') {
      if (!current.empty()) {
        args.push_back(current);
        current = "";
      }
    }
    else {
      current += c;
    }
  }
  if (!current.empty()){
    args.push_back(current);
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
      vector<string> args = splitLine(line);

      for (int i = 0; i < args.size(); ++i) {
        if (i > 0) cout << " ";
        cout << args[i];
      }
      cout << endl;
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
      if (line == "~"){
        chdir(getenv("HOME"));
      }
      else {
        int res = chdir(line.c_str());

        if (res == -1) {
          cout << "cd: " << line << ": No such file or directory" << endl;
        }
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
