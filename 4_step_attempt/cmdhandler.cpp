#include "cmdhandler.h"
#define WRMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH

CmdHandler & CmdHandler::operator= (const CmdHandler & rhs){
  if (this != &rhs){
    char * tmp = stringCopy(rhs.callerDir);
    free(callerDir);
    callerDir = tmp;
    newenvp = rhs.newenvp;
    pwd = rhs.pwd;
    dirStack = rhs.dirStack;
  }
  return *this;
}

void CmdHandler::executeCmd(UserInput * allinp){
  size_t i = 0;
  while(i < allinp->getNumCmd() - 1){
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      perror("pipe");
      break;
    }
    allinp->getNthCmd(i)->changeRedirct(STDOUT, pipefd[1]);
    allinp->getNthCmd(i+1)->changeRedirct(STDIN, pipefd[0]);
    executeSingleCmd(allinp->getNthCmd(i));
    i++;
  }
  executeSingleCmd(allinp->getNthCmd(i));
}

void CmdHandler::executeSingleCmd(SingleCmd * inp){
  std::string cmd(inp->getCmd());
  //inAvailCmd() takes care of "cd" related command
  if(! inAvailCmd(inp)){
    //if the command does not include any path, and not in the available
    //commands list("cd","pushd",etc), then search it in the PATH
    if(! includeSlash(cmd) ){
      char * fullCmdPath = findPath(cmd);
      if(fullCmdPath == NULL){
        std::cout << "Command " << cmd << " not found\n";
        return;
      }
      inp->changeCmd(fullCmdPath);
      delete[] fullCmdPath;
    }
    forkProcess(inp);
  }
}

bool CmdHandler::inAvailCmd(SingleCmd * inp) {
  if( strcmp(inp->getCmd(), "cd") == 0 ){
    chDir(inp->getArgs()[1]);
    return true;
  }
  else if( strcmp(inp->getCmd(), "pushd") == 0 ){
    dirStack.push(pwd);
    chDir(inp->getArgs()[1]);
    return true;
  }
  else if( strcmp(inp->getCmd(), "popd") == 0 ){
    if(dirStack.empty()){
      std::cout << "No directory in stack\n";
    }
    else{
      chDir(dirStack.top().c_str());
      dirStack.pop();
    }
    return true;
  }
  else if( strcmp(inp->getCmd(), "dirstack") == 0 ){
    DirStack dupStack = dirStack;
    DirStack tmp;
    while(! dupStack.empty()){
      tmp.push(dupStack.top());
      dupStack.pop();
    }
    while(! tmp.empty()){
      std::cout << tmp.top() << "\n";
      tmp.pop();
    }
    return true;
  }
  else
    return false;
}

//If no argument is provided, then "cd" == "cd caller's directory"
void CmdHandler::chDir(const char * newDir){
  if(newDir == NULL){
    newDir = callerDir;
  }
  if(chdir(newDir) == -1){
    perror("cd");
  } 
  else{
    char * tmp = get_current_dir_name();
    pwd = std::string(tmp);
    free(tmp);
  }
}

bool CmdHandler::includeSlash(std::string & command) const {
  std::size_t pos = command.find('/');
  if(pos == std::string::npos){
    return false;
  }
  return true;
}
//Return an allocated char *. Caller should delete[] the returned string.
char * CmdHandler::findPath(std::string & cmd){
  char * values = getenv("PATH");
  if(values == NULL) return NULL;
  std::string paths(values);
  //split the paths into vector of strings
  std::vector<std::string> pathVec;
  char delimiter = ':';
  std::size_t end, begin = 0;
  while((end = paths.find_first_of(delimiter, begin)) != std::string::npos){
    pathVec.push_back(paths.substr(begin, end - begin));
    begin = end + 1;
  }
  pathVec.push_back(paths.substr(begin));
  //read each of the directory to find the command
  size_t targetDir = findDir(pathVec, cmd);
  if(targetDir == pathVec.size()){
    return NULL;
  }
  return strToChr(pathVec[targetDir] + "/" + cmd);
}

size_t CmdHandler::findDir(const std::vector<std::string> & pathVec, 
    const std::string & cmd){
  size_t pos = 0;
  while(pos < pathVec.size()){
    DIR * dir = opendir(pathVec[pos].c_str());
    if(dir != NULL){
      struct dirent * dirp;
      while( (dirp = readdir(dir)) ){
        if( cmd.compare(dirp->d_name) == 0){
          closedir(dir);
          return pos;
        }
      }
      closedir(dir);
    }
    pos++; 
  }
  return pos;
}

//forkProcess() uses fork() to run the command specified in "inp", 
//and prints the return status of that command.
//If perror is met for execve(), then it prints that errno instead
//of printing the return status of the child process.
void CmdHandler::forkProcess(SingleCmd * inp){
  pid_t cpid, w;
  int status;

  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    return;
  }
  if (cpid == 0) {            /* Code executed by child */
    runProgram(inp);
  } 
  else {                    /* Code executed by parent */
    do {
      w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        return;
      }
      if (WIFEXITED(status) && WEXITSTATUS(status) != PERROR_MET) {
        std::cout << "Program exited with status " 
                  << WEXITSTATUS(status) << std::endl;
      } else if (WIFSIGNALED(status)) {
        std::cout << "Program was killed by signal " 
                  << WEXITSTATUS(status) << std::endl;
      } else if (WIFSTOPPED(status)) {
        std::cout << "Program was stopped by signal " 
                  << WSTOPSIG(status) << std::endl;
      } else if (WIFCONTINUED(status)) {
        std::cout << "Program continued\n";
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

//runProgram() is a function called by forkProcess(), executed by
//the child process to run the command specified in "inp".
void CmdHandler::runProgram(SingleCmd * inp){
  setRedirct(inp);
  execve(inp->getCmd(), inp->getArgs(), newenvp);
  perror(inp->getCmd());   // execve() only returns on error
  throw execveErr();
}
void CmdHandler::setRedirct(SingleCmd * inp){
  const std::string * redirct = inp->getRedirct();
  if(! redirct[STDIN].empty())
    setRedirct_helper(redirct[STDIN], STDIN, O_RDONLY, WRMODE);
  if(! redirct[STDOUT].empty())
    setRedirct_helper(redirct[STDOUT], STDOUT, O_WRONLY|O_CREAT, WRMODE);
  if(! redirct[STDERR].empty())
    setRedirct_helper(redirct[STDERR], STDERR, O_WRONLY|O_CREAT, WRMODE);
}

void CmdHandler::setRedirct_helper(const std::string & redirctFile, int newfd, 
    int flag, mode_t mode){
  int redirct_fd = open(redirctFile.c_str(), flag, mode);
  if(redirct_fd == -1){
    perror(redirctFile.c_str());
    throw execveErr();
  }
  close(newfd);
  if(dup2(redirct_fd, newfd) == -1){
    perror("dup2");
    throw execveErr();
  }
}

//only for testing
void CmdHandler::printArgs(SingleCmd * inp){
  std::cout << "argv: ";
  for(size_t i = 0; i < inp->getNumArgs(); i++){
    std::cout << inp->getArgs()[i] << "  ";
  }
  std::cout << '\n' << "redirection: ";
  for(size_t i = 0; i < 3; i++){
    std::cout << i << " -- " << inp->getRedirct()[i] << "; ";
  }
  std::cout << std::endl;
}

