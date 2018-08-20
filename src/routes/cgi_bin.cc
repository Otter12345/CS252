#include "http_messages.hh"
#include "misc.hh"
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <link.h>
#include <map>

// You could implement your logic for handling /cgi-bin requests here

typedef void (*httprunfunc)(int ssock, const char* querystring);

HttpResponse handle_cgi_bin(const HttpRequest& request) {
  HttpResponse response;
  response.http_version = request.http_version;
  // TODO: Task 2.2
  setenv("REQUEST_METHOD=GET", "GET", 1);
  std::string str;
  
  //get script
  std::size_t pos = request.request_uri.find_first_of('/');
  std::size_t qMark = request.request_uri.find('?');
  if(qMark == std::string::npos){
	qMark = request.request_uri.length();
  }
  str = request.request_uri.substr(pos, qMark-pos);
 
  //set up QUEST_STRING
  if(request.query != ""){
	setenv("QUERY_STRING", request.query.c_str(), 1);
  }
  
  //loadable modjor
  if(request.request_uri.find(".so") != std::string::npos){
	  httprunfunc httprun;
	  str = "http-root-dir" + str;
	  // Opening 
	  extern std::map<std::string, void *> modulus;
	  std::map<std::string, void *>::iterator it;
	  
	  it = modulus.find(str);
	  void *lib;
	  if(it == modulus.end()){
		  std::cout << "====NEW====" << "\n";
		  lib = dlopen(str.c_str(), RTLD_LAZY);
		  if(lib == NULL){
			  fprintf( stderr, "./hello.so not found\n");
			  perror("dlopen");
			  exit(1);
		  }
		//  const void *libpointer = memmove(lib, libpointer, sizeof(lib));
		  
		 modulus.insert(std::pair<std::string, void *>(str, lib));
	  } else {
		  std::cout << "====PRELOADED====" << "\n";
		  lib = it->second;
	  }
	 
	httprun = (httprunfunc) dlsym(lib, "httprun");
	if(httprun == NULL){
		perror( "dlsym: httprun not found:");
		exit(1);
	}
	
	//create Pipe 
   int pp[2];
   if(pipe(pp)<0) {
	   perror("create pipe");
	   exit(1);
    }
	pid_t pid = fork();
	  if(pid < 0) {
			perror("fork");
			exit(1);
	  }
	  
	if(pid == 0){
		dup2(pp[1], 1);
		close(pp[0]);
		// Call the function
		httprun(1, request.query.c_str());
		exit(0);
	} else{
		  while(waitpid(pid, NULL, 0) > 0){}
		  close(pp[1]);
		  char buf[10000];
		  read(pp[0], buf, sizeof(buf));
		  std::string buffer(buf);
		  
		  //content type
		  std::size_t pos = buffer.find("Content-type: ");
		  if(pos != std::string::npos){
			  int num = strlen("Content-type: ");
			  std::size_t end = buffer.find_first_of("\n");
			  std::string type = buffer.substr(pos + num, end - num);
			  type += ";charset=us-ascii";
			  response.headers.insert(std::pair<std::string, std::string>("Content-type", type));
			  response.message_body = buffer.substr(end+1);
		  }
		  
		  response.headers.insert(std::pair<std::string, std::string>("Connection", "close"));
		  response.status_code = 200;
		  response.reason_phrase = "OK";
		  close(pp[0]);
		  memset(buf, 0, sizeof(buf));
	  }
	  
  } else{
  
	  //create Pipe 
	  int pp[2];
	  if(pipe(pp)<0) {
		  perror("create pipe");
		  exit(1);
	  }
	  
	  pid_t pid = fork();
	  if(pid < 0) {
			perror("fork");
			exit(1);
	  }
	  
	  if(pid == 0){
		  dup2(pp[1], 1);
		  close(pp[0]);
		  str = "http-root-dir" + str;
		  if(execlp(str.c_str(), str.c_str(), (char*)NULL) < 0){
			perror("execlp");
			exit(1);
		  }
		  exit(0);
	  }
  	  else{
		  while(waitpid(pid, NULL, 0) > 0){}
		  close(pp[1]);
		  char buf[10000];
		  read(pp[0], buf, sizeof(buf));
		  std::string buffer(buf);
		  std::size_t pos = buffer.find("Content-type: ");
		  if(pos != std::string::npos){
			  int num = strlen("Content-type: ");
			  std::size_t end = buffer.find_first_of("\n");
			  std::string type = buffer.substr(pos + num, end - num);
			  type += ";charset=us-ascii";
			  response.headers.insert(std::pair<std::string, std::string>("Content-type", type));
			  response.message_body = buffer.substr(end+1);
		  }
		  response.headers.insert(std::pair<std::string, std::string>("Connection", "close"));
		  response.status_code = 200;
		  response.reason_phrase = "OK";
		  close(pp[0]);
		  memset(buf, 0, sizeof(buf));
	  }
  }
  return response;
}

  