#ifndef DOWNLOADRESOURCE_H
#define DOWNLOADRESOURCE_H

#include "Url.h"

using ContentType = std::string;
using ResourceData = std::string;

struct WebRessource
{
    URL url;
    ContentType content_type;
    ResourceData data;
};

struct UnknownProtocol
{
    Protocol protocol;
};

struct ImpossibleAccess
{
    URI uri;
    int error_code;
};


WebRessource downloadFromURL(URL const& url, unsigned int redirect_TTL = 3);

#endif // DOWNLOADRESOURCE_H
