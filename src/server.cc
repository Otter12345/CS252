/**
 * This file contains the primary logic for your server. It is responsible for
 * handling socket communication - parsing HTTP requests and sending HTTP responses
 * to the client. 
 */
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include "server.hh"
#include "socket.hh" 
#include "http_messages.hh"
#include "errors.hh"
#include "misc.hh"
#include "routes.hh"
#include <mutex>
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }

extern "C" void zombie_killer(int signal) {
	int child = wait3(0,0,NULL);
	while((child = waitpid(-1, NULL, WNOHANG)) > 0){}
}

void cleanUpZombie(){
	//clean up child processes
	struct sigaction sa1;
    sa1.sa_handler = zombie_killer;
    sigemptyset(&sa1.sa_mask);
	sa1.sa_flags = SA_RESTART;
    int error = sigaction(SIGCHLD, &sa1, NULL);
	if(error){
		perror("sigaction");
		exit(1);
	}
}

std::mutex mtx;

void Server::run_linear() const {
	cleanUpZombie();
  while (1) {
    Socket_t sock = _acceptor.accept_connection();
    handle(sock);
  }
}

void Server::run_fork() const {
	cleanUpZombie();
  // TODO: Task 1.4
  while(1){
	  Socket_t sock = _acceptor.accept_connection();
	  pid_t pid = fork();
	  if(pid == 0){
		  //child process the request
		  handle(sock);
		  exit(0);
	  }
  }
}

void Server::run_thread() const {
	cleanUpZombie();
  // TODO: Task 1.4
  while(1){
		Socket_t sock = _acceptor.accept_connection();
		if(sock >= 0){
			std::thread t(&Server::handle, this, std::move(sock));
			t.detach();
	}
  }
}

void Server::run_thread_pool(const int num_threads) const {
	cleanUpZombie();
  // TODO: Task 1.4
  while(1){
	std::thread threads[num_threads];
	for(int i=0; i<num_threads; i++){
		threads[i]  = std::thread(&Server::run_linear, this);
	}
	for(int i=0;i<num_threads; i++){
		threads[i].join();
	}
  }
}

// example route map. you could loop through these routes and find the first route which
// matches the prefix and call the corresponding handler. You are free to implement
// the different routes however you please
/*
std::vector<Route_t> route_map = {
  std::make_pair("/cgi-bin", handle_cgi_bin),
  std::make_pair("/", handle_htdocs),
  std::make_pair("", handle_default)
};
*/

std::map<std::string, void *> modulus;

void Server::parse_request(const Socket_t& sock, HttpRequest* request) const {
	/*read line*/
	std::stringstream ss(sock->readline());
	std::string token;
	int flag = 0;

	while(ss >> token){
		if(flag == 0){
			request->method = token;
			flag++;
		} else if(flag == 1){
			request->request_uri = token;
			flag++;
		} else if(flag == 2){
			request->http_version = token;
			flag++;
		} 
	}
	
	std::size_t qMark = request->request_uri.find('?');
	if(qMark != std::string::npos){
		 request->query = request->request_uri.substr(qMark+1);
	}
	/*header*/
	std::string qs = sock->readline();
	flag = 0;
	
	while(qs.compare("\r\n") != 0 && qs.compare("")!=0){
		std::stringstream ss1(qs);
		ss1 >> token;
		if (token == "Authorization:"){
			ss1 >> token;
			ss1 >> token;
			request->headers.insert(std::pair<std::string, std::string>("Authorization:", token));
		} else {
			std::string token1;
			ss1 >> token1;
			request->headers.insert(std::pair<std::string, std::string>(token, token1));
		}
		qs = sock->readline();
	}
	
	//check authorization
	std::map<std::string, std::string>::iterator it;
	it = request->headers.find("Authorization:");
	if(it == request->headers.end()){
		request->headers.insert(std::pair<std::string, std::string>("Authorization:", ""));
	} 
	
	request->message_body = "hello CS252\r\n";
}

void Server::handle(const Socket_t& sock) const {
	mtx.lock();
  HttpRequest request;
  // TODO: implement parsing HTTP requests
  // recommendation:
  parse_request(sock, &request);
  extern int requestNum;
  requestNum++; 
  request.print();
  const std::string ghost = "YW1iZXI6cXdlYXNkenhj";
  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = request.http_version;
  std::map<std::string, std::string>::iterator it;
  it = request.headers.find("Authorization:");
  
  if(it!=request.headers.end()){
	if(it->second == "" || it->second != ghost){
	  resp.status_code = 401;
	  resp.reason_phrase = "Unauthorized";
	  resp.headers.insert(std::pair<std::string, std::string>("WWW-Authenticate", "Basic realm=\"<myhttpd-cs252>\""));
	}else if(it->second == ghost){
		if(request.request_uri.compare("/hello") == 0) {
			resp.status_code = 200;
			resp.reason_phrase = "OK";
			resp.headers.insert(std::pair<std::string, std::string>("Connection", "close"));
			resp.headers.insert(std::pair<std::string, std::string>("Content-Length", "12"));
			resp.message_body = "Hello CS252!\r\n";
		}else {
			if(request.request_uri.find("/cgi-bin") != std::string::npos){
				resp = handle_cgi_bin(request);
			} else { 
				resp = handle_htdocs(request);
			}
		}
	}
  } else {
	  resp.status_code = 401;
	  resp.reason_phrase = "Unauthorized";
	  resp.headers.insert(std::pair<std::string, std::string>("WWW-Authenticate", "Basic realm=\"<myhttpd-cs252>\""));
  }
  std::cout << "Start of HTTP response.." << std::endl;
  extern std::chrono::system_clock::time_point start;
  extern std::vector<std::string> routes;
  extern std::vector<std::string> IPaddress;
  extern std::vector<std::string> respCode;
  extern std::vector<double> runningTime;
  routes.push_back(std::string(request.request_uri));
  respCode.push_back(std::to_string(resp.status_code));
  resp.headers.insert(std::pair<std::string, std::string>("Connection", "close"));
  sock->write(resp.to_string());
  std::chrono::duration<double> temp = std::chrono::system_clock::now() - start;
  runningTime.push_back(temp.count());
  mtx.unlock();
}
