#ifndef ANALYSERESOURCE_H
#define ANALYSERESOURCE_H

#include <set>
#include <unordered_map>

#include "DownloadResource.h"

struct AnalyseResults
{
    std::set<URL> links;
    std::unordered_map<std::string, unsigned int> words;
};

AnalyseResults analyseResource(WebRessource const& res);

#endif // ANALYSERESOURCE_H
