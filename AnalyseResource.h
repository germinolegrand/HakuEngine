#ifndef ANALYSERESOURCE_H
#define ANALYSERESOURCE_H

#include <set>
#include <unordered_map>

#include "DownloadResource.h"

struct AnalyseResults
{
    std::set<URL> links;
    std::unordered_map<std::string, unsigned int> words;
    std::string full_text;
};

AnalyseResults analyseResource(WebRessource const& webres);

#endif // ANALYSERESOURCE_H
