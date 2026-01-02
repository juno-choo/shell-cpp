#include <iostream>
#include <string>
#include <vector>
#include <sstream> // For stringstream (parsing PATH)
#include <unistd.h> // For access(), X_OK, getcwd()
#include <sys/wait.h> // For waitpid
#include <fcntl.h>  // For open(), O_WRONLY, etc.
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
      continue;
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

int redirectStdout(const string& filename) {
  int saved = dup(STDOUT_FILENO);
  int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  dup2(fd, STDOUT_FILENO);
  close(fd);
  return saved;
}

void restoreStdout(int saved) {
  dup2(saved, STDOUT_FILENO);
  close(saved);
}

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
  
  while (true) {
    cout << "$ ";

    string command;
    string line;
    string stdoutFile;

    getline(cin, line); 
    vector<string> tokens = splitLine(line);
    if (tokens.empty()) {
      continue;
    }
    command = tokens[0];

    // Scan for > operator
    for (int i = 0; i < tokens.size(); ++i) {
      if (tokens[i] == ">" || tokens[i] == "1>") {
        // Remove the following tokens so command does not see them as arguments
        // Grab filename
        if (i + 1 < tokens.size()) {
          stdoutFile = tokens[i + 1];
        }
        
        // Erase from i to end
        tokens.erase(tokens.begin() + i, tokens.end());
        break;
      }
    }

    if (command == "exit") {
      break;
    } 

    else if (command == "echo") {
      int saved = -1;

      if (!stdoutFile.empty()) {
        saved = redirectStdout(stdoutFile);
      }

      for (int i = 1; i < tokens.size(); ++i) {
        if (i > 1) cout << " ";
        cout << tokens[i];
      }
      cout << endl;

      if (saved != -1) {
        restoreStdout(saved);
      }
    }
    else if (command == "type") {
      if (tokens.size() < 2) {
        continue;
      }
      if (isBuiltin(tokens[1])) {
        cout << tokens[1] << " is a shell builtin" << endl;
      } 
      else {
        string path = getPath(tokens[1]);
        if (!path.empty()) {
          cout << tokens[1] << " is " << path << endl;
        }
        else {
          cout << tokens[1] << ": not found" << endl;
        }
      }
    }
    else if (command == "pwd") {
      char buffer[1024];
      cout << getcwd(buffer, sizeof(buffer)) << endl;
    }
    else if (command == "cd") {
      // Default home 
      if (tokens.size() < 2 || tokens[1] == "~"){
        chdir(getenv("HOME"));
      }
      else {
        int res = chdir(tokens[1].c_str());

        if (res == -1) {
          cout << "cd: " << tokens[1] << ": No such file or directory" << endl;
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
        vector<string> argParts(tokens.begin() + 1, tokens.end());
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
          // Redirect stdout if needed
          if (!stdoutFile.empty()) {
            int fd = open(stdoutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
          }

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
