#ifndef ANALYSERESOURCE_H
#define ANALYSERESOURCE_H

#include <vector>
#include <unordered_map>

#include "DownloadResource.h"

struct AnalyseResults
{
    std::vector<URL> links;
    std::unordered_map<std::string, unsigned int> words;
};

AnalyseResults analyseResource(WebRessource const& res);

#endif // ANALYSERESOURCE_H
