#include "userinput.h"
#include "cmdhandler.h"
#include <string.h>
#include <iostream>

UserInput::UserInput(const std::string & allinput): 
    allCmd(std::vector<SingleCmd>()){
  std::string vline(" | ");
  std::string tmpstr;
  std::size_t begin = 0, end = 0;
  while( (end = allinput.find_first_of(vline,begin)) 
      != std::string::npos){
    tmpstr = allinput.substr(begin, end - begin);
    allCmd.push_back(SingleCmd(tmpstr));
    begin = end + vline.size();
  }
  tmpstr = allinput.substr(begin);
  allCmd.push_back(SingleCmd(tmpstr));
}

SingleCmd::SingleCmd(): originalInput(std::string()), 
    args(NULL), num_arg(0), redirct(NULL){
  args = new char*[1];
  args[0] = NULL;
  redirct = new std::string[3];
  for(size_t i = 0; i < 3; i++){
    redirct[i] = std::string();
  }
}

SingleCmd::SingleCmd(std::string & input): originalInput(input), 
    args(NULL), num_arg(0), redirct(NULL){

  redirct = new std::string[3];
  for(size_t i = 0; i < 3; i++){
    redirct[i] = std::string();
  }
  parseInput();
  if(strcmp(args[0], "exit") == 0) {
    destroy();
    throw callExit();
  }
}

SingleCmd::SingleCmd(const SingleCmd & rhs): originalInput(rhs.originalInput),  
    args(NULL), num_arg(rhs.num_arg), redirct(new std::string[3]()){
  *this = rhs;
}

SingleCmd & SingleCmd::operator= (const SingleCmd & rhs){
  if (this != &rhs){
    char ** tmpargs = new char*[rhs.num_arg + 1];
    for(size_t i = 0; i < rhs.num_arg; i++){
      tmpargs[i] = stringCopy(rhs.args[i]);
    }
    tmpargs[rhs.num_arg] = NULL;
    std::string * tmpredirct = new std::string[3]();
    for(size_t i = 0; i < 3; i++){
      tmpredirct[i] = rhs.redirct[i];
    }
    destroy();
    originalInput = rhs.originalInput;
    num_arg = rhs.num_arg;
    args = tmpargs;
    redirct = tmpredirct;
  }
  return *this;
}

void SingleCmd::parseInput(){
  std::vector<std::string> argsVec;
  std::string whitespaces(" ");//add any other whitespace chars if desired
  std::size_t begin = 0, end = 0;
  while( (begin = originalInput.find_first_not_of(whitespaces,end)) 
      != std::string::npos){
    num_arg++;
    end = findEnd(begin, originalInput, whitespaces);
    if(end == std::string::npos){
      argsVec.push_back(originalInput.substr(begin));
      break;
    }
    argsVec.push_back(originalInput.substr(begin, end - begin));
  }
  if(num_arg == 0){
    destroy();
    throw emptyORbadInput();
  }
  findRedirct(argsVec);
  args = vecToArr(argsVec);
}

//take care of "\ "
std::size_t SingleCmd::findEnd(std::size_t begin, std::string & input, 
    std::string & whitespaces){
  std::size_t end = input.find_first_of(whitespaces, begin);
  if(end != std::string::npos && input[end - 1] == '\\'){
    input.erase(end - 1, 1);
    return findEnd(end, input, whitespaces);
  }
  return end;
}

void SingleCmd::findRedirct(std::vector<std::string> & argsVec){
  std::vector<std::string>::iterator it = argsVec.begin();
  while(it != argsVec.end()){
    if(it->compare("<") == 0){ //stdin
      redirctOp(argsVec, it, STDIN);
    } 
    else if(it->compare(">") == 0){ 
      redirctOp(argsVec, it, STDOUT);
    }
    else if(it->compare("2>") == 0){ 
      redirctOp(argsVec, it, STDERR);
    }
    else{
      ++it;
    }
  }
}
void SingleCmd::redirctOp(std::vector<std::string> & argsVec, 
    std::vector<std::string>::iterator & it, int fileDescpt){
  if(it + 1 == argsVec.end()){
    std::cout << "Syntax error: no filename provided for redirection.\n";
    destroy();
    throw emptyORbadInput();
  }
  else{
    if(it == argsVec.begin()){
      redirct[fileDescpt] = *(it + 1);
      argsVec.erase(it, it + 2);
      num_arg -= 2;
      it = argsVec.begin();
    }
    else{
      std::vector<std::string>::iterator it_prev = it - 1;
      redirct[fileDescpt] = *(it + 1);
      argsVec.erase(it, it + 2);
      num_arg -= 2;
      it = it_prev + 1;
    }
  }
}

void SingleCmd::destroy(){
  if(args != NULL){
    for(size_t i = 0; i < num_arg + 1; i++){
      delete[] args[i];
    }
    delete[] args;
  }
  delete[] redirct;
}

char * stringCopy(const char * src){
  if(src == NULL) return NULL;
  std::size_t strlength = strlen(src); 
  char * cpy = new char[strlength + 1];
  strcpy(cpy, src);
  cpy[strlength] = '\0';
  return cpy;
}
char * strToChr(const std::string & stdstr){
  char * chr = new char[stdstr.length() + 1];
  for(size_t i = 0; i < stdstr.length(); i++){
    chr[i] = stdstr[i];
  }
  chr[stdstr.length()] = '\0';
  return chr;
}
char ** vecToArr(const std::vector<std::string> & argsVec){
  char ** argsArr = new char*[argsVec.size() + 1];
  for(size_t i = 0; i < argsVec.size(); i++){
    argsArr[i] = strToChr(argsVec[i]);
  }
  argsArr[argsVec.size()] = NULL;
  return argsArr;
}

