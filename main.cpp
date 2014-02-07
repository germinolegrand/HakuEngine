#include <iostream>
#include "tinyxml2/tinyxml2.h"

#include "DownloadResource.h"


int main(int argc, char* argv[])
{
    std::cout << "Hello Miyazaki's world!" << std::endl;

//    std::string resource = downloadURI(uri);
//    AnalyseResults analyseResults = analyseResource(uri, resource);
//    persistAnalyseResults(uri, analyseResults);

    WebRessource indexHaku = downloadFromURL("http://192.168.1.36/");

    std::cout << indexHaku.data;

//    tinyxml2::XMLDocument xmldoc;
//    xmldoc.Parse(response.getBody().c_str(), response.getBody().size());

    return 0;
}
