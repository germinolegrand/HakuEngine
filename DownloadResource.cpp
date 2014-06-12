#include "DownloadResource.h"

#include <chrono>
#include <iostream>
#include <SFML/Network.hpp>

struct Redirected
{
    URL url;
};


const size_t download_HTTP_content_type = 0;
const size_t download_HTTP_content_data = 1;
std::tuple<ContentType, ResourceData> download_HTTP(Host const& host, URI const& uri, std::chrono::milliseconds timeout)
{
    sf::Http http;
    http.setHost(host);

    sf::Http::Request request;
    request.setMethod(sf::Http::Request::Get);
    request.setUri(uri);
    request.setHttpVersion(1, 1); // HTTP 1.1

    sf::Http::Response response = http.sendRequest(request, sf::milliseconds(timeout.count()));

    //handle status and throw errors
    sf::Http::Response::Status status = response.getStatus();

    if(status == sf::Http::Response::Ok)
    {
        return std::make_tuple(response.getField("Content-Type"), response.getBody());
    }
    else if(status == sf::Http::Response::MovedTemporarily || status == sf::Http::Response::MovedPermanently)
    {
        throw Redirected{response.getField("Location")};
    }

    throw ImpossibleAccess{"http://" + host + uri, status};
}

WebRessource downloadFromURL(URL const& url, unsigned int redirect_TTL)
{
    if(redirect_TTL == 0)
    {
        throw ImpossibleAccess{url};
    }

    WebRessource web_res;
    web_res.url = url;

    Protocol protocol = protocol_of_url(url);

    if(protocol == "http")
    {
        try
        {
            auto download_result = download_HTTP(host_of_url(url), uri_of_url(url), std::chrono::milliseconds(2000));

            web_res.content_type = std::move(std::get<download_HTTP_content_type>(download_result));
            web_res.data         = std::move(std::get<download_HTTP_content_data>(download_result));
        }
        catch(Redirected const& e)
        {
            return downloadFromURL(e.url, redirect_TTL - 1);
        }
    }
    else if(protocol == "ftp")
    {
        sf::Ftp ftp;

        auto res = ftp.connect(/*"ftp://" +*/ host_of_url(url));
        if(!res.isOk())
            throw ImpossibleAccess{url, res.getStatus()};

        res = ftp.login();
        if(!res.isOk())
            throw ImpossibleAccess{url, res.getStatus()};

        URI uri = uri_of_url(url);

        auto listing_res = ftp.getDirectoryListing(uri);

        if(!listing_res.isOk())
            throw ImpossibleAccess{url, listing_res.getStatus()};

        std::cout << listing_res.getMessage() << std::endl;

        for(auto& entry : listing_res.getListing())
            std::cout << entry << std::endl;

        std::cout << "\n" << uri << std::endl;

        if(std::find(begin(listing_res.getListing()), end(listing_res.getListing()), uri) == end(listing_res.getListing()))
            throw ImpossibleAccess{url, 0};

        web_res.content_type = "ftp/file";
    }
    else
    {
        throw UnknownProtocol{protocol};
    }

    return web_res;
}
