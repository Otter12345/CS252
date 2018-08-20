#include "http_messages.hh"

// If you use the recommended route handling scheme, you will probably want a default handler

HttpResponse handle_default(const HttpRequest& request) {
  HttpResponse response;
  response.http_version = request.http_version;
  response.status_code = 404;
  return response;
}

