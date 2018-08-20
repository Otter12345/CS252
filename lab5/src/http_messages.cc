#include "http_messages.hh"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
// You may find this map helpful. You can implement HttpResponse::to_string() such that
// if no reason_phrase is set, then you try looking up a default_status_reason in this
// std::map (dictionary). These codes are copied from RFC2616 Sec. 6.1.1
const std::map<const int, const std::string> default_status_reasons = {
    {100, "Continue"}, {101, "Switching Protocols"},
    {200, "OK"}, {201, "Created"}, {202, "Accepted"}, {203, "Non-Authoritative Information"},
    {204, "No Content"}, {205, "Reset Content"}, {206, "Partial Content"},
    {300, "Multiple Choices"}, {301, "Moved Permanently"}, {302, "Found"}, {303, "See Other"},
    {304, "Not Modified"}, {305, "Use Proxy"}, {307,  "Temporary Redirect"}, {400, "Bad Request"},
    {401, "Unauthorized"}, {402, "Payment Required"}, {403, "Forbidden"}, {404, "Not Found"},
    {405, "Method Not Allowed"}, {406, "Not Acceptable"}, {407, "Proxy Authentication Required"},
    {408, "Request Time-out"}, {409, "Conflict"}, {410, "Gone"}, {411, "Length Required"},
    {412, "Precondition Failed"}, {413, "Request Entity Too Large"},
    {414, "Request-URI Too Large"}, {415,  "Unsupported Media Type"},
    {416, "Requested range not satisfiable"}, {417, "Expectation Failed"},
    {500, "Internal Server Error"}, {501, "Not Implemented"}, {502, "Bad Gateway"},
    {503, "Service Unavailable"}, {504, "Gateway Time-out"}, {505, "HTTP Version not supported"}
};

std::string HttpResponse::to_string() const {
    // TODO: Create a valid HTTP response string from the structure
    std::stringstream ss;
    // The following is an example of how to use stringstream.
    // You should remove all of this and create a valid HTTP response
    // message based on the variables defined in http_messages.hh

    // Look at RFC 2616 Section 6 for details on how a response message looks:
    // https://tools.ietf.org/html/rfc2616#section-6
	ss << http_version << " " << std::to_string(status_code) << " " << reason_phrase << "\r\n";
	/*std::map<std::string, std::string>::iterator iterator;
	iterator = headers.begin();
	headers.insert(iterator, std::pair<std::string, std::string>("Connection", "close"));*/
	for(auto& it:headers){
		ss << it.first << ": " << it.second << "\r\n";
	}
	ss << "\r\n" << message_body;
	//update log 
	  extern std::vector<std::string> routes;
	  extern std::vector<std::string> IPaddress;
	  extern std::vector<std::string> respCode;
	  /*
	  int fd = open("myhttpd.log", O_RDONLY, 0);
	  struct stat st;
	  stat("myhttpd.log", &st);
	  void *mmappedData = mmap(NULL, st.size, ORIT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
	  */
	  
	std::ofstream file("myhttpd.log", std::ios_base::app);
	if(file){
	//std::cout << IPaddress.size() << " " << routes.size() << " " << respCode.size() << std::endl;
		std::stringstream ss;
		for(int i=0; i<routes.size(); i++){
		if(IPaddress.size()!=0)
			ss << IPaddress.at(IPaddress.size()-1) << '\t' ;
		else 
			ss << "\t\t";
		if(routes.size()!=0)
			ss << routes.at(i) << '\t' ;
		else 
			ss << "\t\t";
		if(respCode.size()!=0)
			ss << respCode.at(i); 
		else 
			ss << "\t\t";
		}
		ss << std::endl;
		file << ss.rdbuf();
		file.close();
	}
    return ss.str();
}

void HttpRequest::print() const {
    // Magic string to help with autograder
	extern time_t totalTime;
	
    std::cout << "\\\\==////REQ\\\\\\\\==////" << std::endl;

    std::cout << "Method: {" << method << "}" << std::endl;
    std::cout << "Request URI: {" << request_uri << "}" << std::endl;
    std::cout << "Query string: {" << query << "}" << std::endl;
    std::cout << "HTTP Version: {" << http_version << "}" << std::endl;

    std::cout << "Headers: " << std::endl;
    for (auto kvp=headers.begin(); kvp != headers.end(); kvp++) {
        std::cout << "field-name: " << kvp->first << "; field-value: " << kvp->second << std::endl;
    }

    std::cout << "Message body length: " << message_body.length() << std::endl <<
      message_body << std::endl;

    // Magic string to help with autograder
    std::cout << "//==\\\\\\\\REQ////==\\\\" << std::endl;
}
