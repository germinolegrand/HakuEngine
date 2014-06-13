#ifndef ANALYSERESOURCE_H
#define ANALYSERESOURCE_H

#include <set>
#include <vector>
#include <unordered_map>
#include <functional>

#include "DownloadResource.h"

#include "gumbo-parser-master/src/gumbo.h"

struct AnalyseResults
{
    std::set<URL> links;
    std::unordered_map<std::string, URL> backlinks;
    std::unordered_map<std::string, unsigned int> words;
    std::string full_text;
};

struct UnknownContentType
{
    ContentType content_type;
};

using ContentAnalyser = std::function<AnalyseResults(WebRessource const&)>;

class Analyzer
{
public:
    Analyzer() = default;

    AnalyseResults operator()(WebRessource const& webres);

    void setAnalyser(ContentType const& content_type, ContentAnalyser analyser);

private:
    std::unordered_map<ContentType, ContentAnalyser> m_analysers;
};

using HTMLAnalyser = std::function<void(WebRessource const&, AnalyseResults&, GumboNode*)>;

class TextHTMLAnalyser
{
public:
    TextHTMLAnalyser() = default;

    AnalyseResults operator()(WebRessource const& webres);

    void addAnalyser(HTMLAnalyser analyser);

private:
    std::vector<HTMLAnalyser> m_analysers;

    void analyse_node(WebRessource const& webres, AnalyseResults& results, GumboNode* node);
};

void search_for_text(WebRessource const& webres, AnalyseResults& results, GumboNode* node);
void search_for_links(WebRessource const& webres, AnalyseResults& results, GumboNode* node);
void search_for_words(WebRessource const& webres, AnalyseResults& results, GumboNode* node);
std::function<bool(WebRessource const& webres, AnalyseResults& results, GumboNode* node)>
generate_is_inside_tag(GumboTag tag);
std::function<void(WebRessource const& webres, AnalyseResults& results, GumboNode* node)>
generate_skip_tag(GumboTag tag);

AnalyseResults analyse_ftp_file(WebRessource const& webres);

#endif // ANALYSERESOURCE_H
