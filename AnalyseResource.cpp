#include "AnalyseResource.h"

#include "gumbo-parser-master/src/gumbo.h"
#include "utils.h"
#include "Url.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

class SkipThisNode{};
class InvalidURL{};
class SkipThisWord{};

std::string link_to_full_url(URL const& url, std::string link)
{
    if(protocol_of_url(link).size() == link.size())
    {
        if(link.size() >= 1 && link[0] == '/')
            link = link.substr(1);

        auto host = host_of_url(url);

        if(host.size() >= 1 && host[host.size() - 1] != '/')
            host += '/';

        link = protocol_of_url(url) + "://" + host + link;
    }

    ///Delete anything after #
    {
        size_t url_end = link.find('#');

        if(url_end != std::string::npos)
            link.resize(url_end);
    }

//    ///Delete anything after ?
//    {
//        size_t url_end = link.find('?');
//
//        if(url_end != std::string::npos)
//            link.resize(url_end);
//    }

    return link;
}

std::string normalize_word(std::string word)
{
    ///TODO trouver pourquoi ça bouffe la dernière lettre !
    if(word.size() >= 1 && std::string(",.").find(word[word.size() - 1]) != std::string::npos)
        word.resize(word.size() - 1);

    if(word.size() < 3)
        throw SkipThisWord();

    return word;
}

void skip_style_tag(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_STYLE)
        throw SkipThisNode{};
}

void skip_script_tag(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_SCRIPT)
        throw SkipThisNode{};
}

void search_for_words(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    int score = 1;

    ///Score up the <STRONG>
    {
        static thread_local GumboNode* strong_node = nullptr;
        static thread_local unsigned int strong_node_children_countdown = 0;

        if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_STRONG)
        {
            strong_node_children_countdown = node->v.element.children.length;

            if(strong_node_children_countdown > 0)
                strong_node = node;

            return;
        }

        if(strong_node)
            ++score;

        if(strong_node && node->parent == strong_node)
        {
            --strong_node_children_countdown;

            if(strong_node_children_countdown == 0)
                strong_node = nullptr;
        }
    }

    if(node->type != GUMBO_NODE_TEXT)
        return;

    std::string text(node->v.text.text);

    for(auto beg = begin(text),
              en = end(text),
              it = begin(text)
        ; it != en ;)
    {
        beg = std::find_if(it, en, utils::isnotgraph);
        it = std::find_if(beg, en, utils::isgraph);

        try
        {
            auto pair_it_bool = results.words.emplace(normalize_word({beg, it}), score);

            if(!pair_it_bool.second)
                pair_it_bool.first->second += score;
        }
        catch(SkipThisWord const& e)
        {}
    }
}

void search_for_links(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    if(node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboAttribute* href;
    if(node->v.element.tag == GUMBO_TAG_A && (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
    {
        try
        {
            results.links.insert(link_to_full_url(webres.url, href->value));
        }
        catch(InvalidURL const&)
        {}
    }
}

void search_for_text(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    if(node->type != GUMBO_NODE_TEXT)
        return;

    std::string text(node->v.text.text);

    if(node->index_within_parent != 0)
        results.full_text.append(" ");

    results.full_text.append(text);
}

void analyseNode(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    skip_script_tag(webres, results, node);
    skip_style_tag(webres, results, node);
    search_for_links(webres, results, node);
    search_for_words(webres, results, node);
    search_for_text(webres, results, node);

    if(node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboVector* children = &node->v.element.children;
    for(unsigned int i = 0; i < children->length; ++i)
    {
        try
        {
            analyseNode(webres, results, static_cast<GumboNode*>(children->data[i]));
        }
        catch(SkipThisNode const&){}
    }
}

AnalyseResults analyseResource(WebRessource const& webres)
{
    AnalyseResults results;

    GumboOutput* output = gumbo_parse(webres.data.c_str());

    AT_SCOPE_EXIT([output]{
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    });

    analyseNode(webres, results, output->root);

    return results;
}
