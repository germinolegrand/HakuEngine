#include "DownloadResource.h"

#include <SFML/Network.hpp>

const size_t download_HTTP_content_type = 0;
const size_t download_HTTP_content_data = 1;
std::tuple<ContentType, ResourceData> download_HTTP(Host const& host, URI const& uri)
{
    sf::Http http;
    http.setHost(host);

    sf::Http::Request request;
    request.setMethod(sf::Http::Request::Get);
    request.setUri(uri);
    request.setHttpVersion(1, 1); // HTTP 1.1

    sf::Http::Response response = http.sendRequest(request, sf::milliseconds(10000));

    sf::Http::Response::Status status = response.getStatus();

    //TODO handle status and throw errors

    return std::make_tuple(response.getField("Content-Type"), response.getBody());
}

Protocol protocol_of_url(URL const& url)
{
    return url.substr(0, url.find("://"));
}

Host host_of_url(URL const& url)
{
    unsigned int host_begin = url.find("://") + 3;
    return url.substr(host_begin, url.find('/', host_begin));
}

URI uri_of_url(URL const& url)
{
    return url.substr(url.find('/', url.find("://") + 3));
}

WebRessource downloadFromURL(URL const& url)
{
    WebRessource web_res;
    web_res.url = url;

    Protocol protocol = protocol_of_url(url);

    if(protocol == "http")
    {
        auto download_result = download_HTTP(host_of_url(url), uri_of_url(url));

        web_res.content_type = std::move(std::get<download_HTTP_content_type>(download_result));
        web_res.data         = std::move(std::get<download_HTTP_content_data>(download_result));
    }
    else if(protocol == "ftp")
    {
        //TODO download from FTP
    }
    else
    {
        //TODO throw unknown protocol
    }

    return web_res;
}
