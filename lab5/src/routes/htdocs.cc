#include "http_messages.hh"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <experimental/filesystem>
#include <time.h>
#include "misc.hh"
#include "routes.hh"
#include <chrono>
#include <ctime>
#include <numeric>
// You may find implementing this function and using it in server.cc helpful

HttpResponse handle_htdocs(const HttpRequest& request) {
  HttpResponse response;
  response.http_version = request.http_version;
  // TODO: Task 1.3
  // Get the request URI, verify the file exists and serve it
  struct stat sb;
  std::string uri;
  bool flag = false;
  bool dirFlag = false;
  //check if it is a file
  if(request.request_uri.find("/index.html" ) != std::string::npos){
	  uri = "http-root-dir/htdocs" + request.request_uri;
  } else if(request.request_uri.length() != 1 && request.request_uri.back() == '/'){
	  uri = "http-root-dir/htdocs" + request.request_uri;
	  dirFlag = true;
  } else if(request.request_uri.find("/stats")!= std::string::npos){
	  uri = "http-root-dir/htdocs" + request.request_uri;
	  flag = true;
  } else if(request.request_uri.find("/logs") != std::string::npos){
	  uri = "myhttpd.log";
  } else {
	  if(!strcmp(request.request_uri.c_str(), "/")){uri = "http-root-dir/htdocs" + request.request_uri + "index.html";}
	  else uri = "http-root-dir/htdocs" + request.request_uri;
  }

  if(flag){
	  extern int requestNum;
	  extern std::chrono::system_clock::time_point start;
	  extern std::vector<double> runningTime;
	  extern std::vector<std::string> routes;
	  extern std::vector<std::string> IPaddress;
	  extern std::vector<std::string> respCode;
	  routes.push_back(std::string(request.request_uri));
	  respCode.push_back(std::to_string(200));
	  std::chrono::duration<double> temp = std::chrono::system_clock::now() - start;
	  runningTime.push_back(temp.count());
	  std::stringstream ss;
	  ss << "Xu He" << std::endl;
	  double totalTime = 0;
	  totalTime = std::accumulate(runningTime.begin(), runningTime.end(), 0.0);
	  ss << "The uptime of the server (in sec): " << totalTime << std::endl;
	  ss << "The number of requests so far: " << requestNum << std::endl;
	  ss << "maximum service time (in sec): " << *max_element(runningTime.begin(), runningTime.end()) 
	  << '\t' << "URL: " << routes.at(max_element(runningTime.begin(), runningTime.end()) - runningTime.begin()) << std::endl;
	  ss << "minimum service time (in sec): " << *min_element(runningTime.begin(), runningTime.end())
	  << '\t' << "URL: " << routes.at(min_element(runningTime.begin(), runningTime.end()) - runningTime.begin()) << std::endl;
	  response.message_body = ss.str();
  } else {
	  //check if file/dir exists
	  if(stat(uri.c_str(), &sb) == -1){
		  response.status_code = 404;
		  response.reason_phrase = "Not Found!";
		  response.message_body = "Not Found!";
		  return response;
	  }

   if(sb.st_mode & S_IFDIR) {
	   if(dirFlag){
		    DIR *dir = opendir(uri.c_str());
			if(dir == NULL){
				response.status_code = 404;
				response.reason_phrase = "Not Found!";
				response.message_body = "Not Found!";
				return response;
			}
			struct dirent *ent;
			std::stringstream ss1;
			ss1 << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">" << std::endl;
			ss1 << "<html>" << std::endl;
			ss1 << "<body>" << std::endl;
			ss1 << "<ul>" << std::endl;
			while((ent = readdir(dir)) != NULL) {
				if((strcmp(ent->d_name, ".")!= 0) && (strcmp(ent->d_name, "..")!=0)){
					std::string str = request.request_uri + ent->d_name;
					str = "<li><a href=\"" + str + "\">" + ent->d_name + "</a>";
					ss1 << str << std::endl;
				}
			}
			ss1 << "</ul>" << std::endl;
			ss1 << "</body>" << std::endl;
			ss1 << "</html>" << std::endl;
			response.message_body = ss1.str();
			closedir(dir);
	   } else {
			DIR *dir;
			struct dirent *ent;
		if((dir = opendir(uri.c_str())) != NULL) {
			uri += "/index.html";
			std::ifstream infile(uri.c_str(), std::ios::binary);
			if(!infile.is_open()){
				response.status_code = 404;
				response.reason_phrase = "Not Found!";
				response.message_body = "Not Found!";
				return response;
			}else{
				response.headers.insert(std::pair<std::string, std::string>("Content-type", get_content_type(uri)));
				std::stringstream ss;
				ss << infile.rdbuf();
				response.headers.insert(std::pair<std::string, std::string>("Content-Length", std::to_string(sb.st_size)));
				response.message_body = ss.str();
				infile.close();
			}
				closedir(dir);
			} else{
				response.status_code = 404;
				response.reason_phrase = "Not Found!";
				response.message_body = "index.html Not Found!";
				return response;
			}
	   }	
	} else if(sb.st_mode & S_IFREG){
		//handle_readFile(uri, response, sb.st_size);
		std::ifstream infile(uri.c_str(), std::ios::binary);
		if(!infile.is_open()){
			response.status_code = 404;
			response.reason_phrase = "Can't open file!";
			response.message_body = "Not Found!";
			return response;
		}else{
			response.headers.insert(std::pair<std::string, std::string>("Content-type", get_content_type(uri)));
			std::stringstream ss;
			ss << infile.rdbuf();
			response.headers.insert(std::pair<std::string, std::string>("Content-Length", std::to_string(sb.st_size)));
			response.message_body = ss.str();
			infile.close();
		}
	}
  }
  
  	response.status_code = 200;
	response.reason_phrase = "OK";
	response.headers.insert(std::pair<std::string, std::string>("Connection", "close"));
  return response;
}
