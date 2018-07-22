#include <cstdlib>
#include <iostream>
#include <string>
#include <cstddef>

#include "userinput.h"
#include "cmdhandler.h"

int main(int argc, char * argv[], char * envp[]){
  std::string input;
  UserInput inp;
  CmdHandler cmdHandler(envp);
  cmdHandler.printPrompt();
  while(getline(std::cin, input)){
    try{
      inp = UserInput(input);
    }
    catch(emptyORbadInput & e){
      cmdHandler.printPrompt();
      continue;
    }
    catch(callExit & e){
      break;
    }
//    cmdHandler.printArgs(&inp); //testing
    try{
      cmdHandler.executeCmd(&inp);
    }
    catch(execveErr & e){ //For child process if execve() fails
      return PERROR_MET;
    }
    cmdHandler.printPrompt();
  }
  return EXIT_SUCCESS;
}
