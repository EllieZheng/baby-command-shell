#ifndef _CMD_HANDLER_H_
#define _CMD_HANDLER_H_

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <stack>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "userinput.h"
#define PERROR_MET 127

//Caller should catch execveErr in case execve() fails in the child process
class execveErr : public std::exception {}; 

class CmdHandler{
private:
  typedef std::stack<std::string> DirStack;
  char ** newenvp;
  char * callerDir;
  std::string pwd;
  DirStack dirStack;
  bool includeSlash(std::string & command) const;
  bool inAvailCmd(UserInput * inp);
  char * findPath(std::string & cmd);
  size_t findDir(const std::vector<std::string> & pathVec, 
      const std::string & cmd);
  void chDir(const char* newDir);
  void forkProcess(UserInput * inp);
  void runProgram(UserInput * inp);
  void setRedirct(UserInput * inp);
  void setRedirct_helper(const std::string & redirctFile, int newfd, 
      int flag, mode_t mode);

public:
  CmdHandler() : 
      newenvp(NULL), callerDir(get_current_dir_name()), 
      pwd(std::string(callerDir)), dirStack(DirStack()){}
  explicit CmdHandler(char * envp[]): 
      newenvp(envp), callerDir(get_current_dir_name()), 
      pwd(std::string(callerDir)), dirStack(DirStack()){}
  ~CmdHandler(){ free(callerDir); }
  CmdHandler(const CmdHandler & rhs):
      newenvp(rhs.newenvp), callerDir(stringCopy(rhs.callerDir)), 
      pwd(rhs.pwd), dirStack(rhs.dirStack){}
  CmdHandler & operator= (const CmdHandler & rhs);

  void printPrompt(){ std::cout << "myShell:" << pwd << " $ "; }
  //executeCmd() either executes the "cd"-related commands
  //(Note: "cd" by default changes dir into caller's directory)
  //or fork the process to execute other specified programs(if found)
  void executeCmd(UserInput * inp);
  void printArgs(UserInput * inp);//used for testing
};

#endif
