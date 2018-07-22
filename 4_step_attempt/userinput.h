#ifndef _USER_INPUT_H_
#define _USER_INPUT_H_ 

#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <exception>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define STDIN 0
#define STDOUT 1
#define STDERR 2


//deep copy of a C string. Caller should delete[] the returned str.
char * stringCopy(const char * src);
//convert std::string to a C string. Caller should delete[] the returned str.
char * strToChr(const std::string & stdstr);
//convert a vector of std::string to an array of C string. Null terminated. 
//Caller should delete[] the returned array and its elements.
char ** vecToArr(const std::vector<std::string> & argsVec);

//To handle special input. Catch them after calling the SingleCmd constructor.
class emptyORbadInput : public std::exception {};
class callExit: public std::exception {};

//store and deal with user's input
class SingleCmd{
private:
  std::string originalInput;
  char ** args; //argument lists, including command
  size_t num_arg; //excluding NULL
  std::string * redirct; //store redirct[file descriptor] = filename for redirection

  void parseInput();
  std::size_t findEnd(std::size_t begin, std::string & input, 
      std::string & whitespaces);
  void findRedirct(std::vector<std::string> & argsVec);
  void redirctOp(std::vector<std::string> & argsVec, std::vector
      <std::string>::iterator & it, int fileDescpt);
  void destroy();

public:
  //the constructor parses the user's input
  SingleCmd(); 
  explicit SingleCmd(std::string & input);
  SingleCmd(const SingleCmd & rhs);
  ~SingleCmd(){ destroy(); }
  SingleCmd & operator= (const SingleCmd & rhs);
  char * getCmd() const { return args[0]; }
  void changeCmd(const char * fullCmd){
    delete[] args[0]; 
    args[0] = stringCopy(fullCmd);
  }
  char ** getArgs() const { return args; }
  size_t getNumArgs() const { return num_arg; }
  std::string * getRedirct() const { return redirct; }
  void changeRedirct(int fd, int pipefd){ redirct[fd] = pipefd; }
};
 
class UserInput{
private:
  std::vector<SingleCmd> allCmd;
public:
  UserInput() : allCmd(std::vector<SingleCmd>()){};
  explicit UserInput(const std::string & allinput);
  ~UserInput(){}
  size_t getNumCmd(){ return allCmd.size(); }
  SingleCmd * getNthCmd(size_t n){ return &(allCmd[n]); }
};

#endif
