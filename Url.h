#ifndef URL_H
#define URL_H

#include <string>

using URL = std::string;
using Protocol = std::string;
using Host = std::string;
using Port = unsigned short;
using URI = std::string;

inline Protocol protocol_of_url(URL const& url)
{
    unsigned int protocol_end = url.find("://");

    if(protocol_end == std::string::npos)
        return "";

    return url.substr(0, protocol_end);
}

inline Host host_of_url(URL const& url)
{
    unsigned int host_begin = url.find("://") + 3;

    if(host_begin == std::string::npos + 3)
        return "";

    unsigned int host_end = url.find('/', host_begin);

    return url.substr(host_begin, host_end - host_begin);
}

inline URI uri_of_url(URL const& url)
{
    unsigned int uri_begin = url.find('/', url.find("://") + 3);
    return uri_begin < url.size() ? url.substr(uri_begin) : "";
}

#endif // URL_H
