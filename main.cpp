#include <SFML/Network.hpp>
#include <iostream>
#include "tinyxml2/tinyxml2.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;


    sf::Http http;
    http.setHost("http://192.168.1.36/");

    sf::Http::Request request;
    request.setMethod(sf::Http::Request::Get);
    request.setUri("/info.php");
    request.setHttpVersion(1, 1); // HTTP 1.1

    sf::Http::Response response = http.sendRequest(request);
    std::cout << "status: " << response.getStatus() << std::endl;
    std::cout << "HTTP version: " << response.getMajorHttpVersion() << "." << response.getMinorHttpVersion() << std::endl;
    std::cout << "Content-Type header:" << response.getField("Content-Type") << std::endl;
    std::cout << "body: " << response.getStatus() << std::endl;
    std::cout << response.getBody() << std::endl;

    tinyxml2::XMLDocument xmldoc;
    xmldoc.Parse(response.getBody().c_str(), response.getBody().size());

    return 0;
}
