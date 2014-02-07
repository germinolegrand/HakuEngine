#ifndef DOWNLOADRESOURCE_H
#define DOWNLOADRESOURCE_H

#include <string>

using URL = std::string;
using Protocol = std::string;
using Host = std::string;
using Port = unsigned short;
using URI = std::string;
using ContentType = std::string;
using ResourceData = std::string;

struct WebRessource
{
    URL url;
    ContentType content_type;
    ResourceData data;
};

WebRessource downloadFromURL(URL const& url);

#endif // DOWNLOADRESOURCE_H
