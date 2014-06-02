#include "AnalyseResource.h"

#include "gumbo-parser-master/src/gumbo.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>


void search_for_words(AnalyseResults& results, GumboNode* node)
{
    if(node->type != GUMBO_NODE_TEXT)
        return;

    GumboText* gtext = &node->v.text;
    std::string text(gtext->text);

    for(auto beg = begin(text),
              en = end(text),
              it = std::find_if(begin(text), end(text), std::isblank)
        ; beg != it && it != en
        ; beg = ++it, it = std::find_if(it, en, std::isblank))
    {
        std::string value(beg, it);
        auto pair_it_bool = results.words.emplace(std::move(value), 1);

        if(!pair_it_bool.second)
            ++pair_it_bool.first->second;
    }
}

void search_for_links(AnalyseResults& results, GumboNode* node)
{
    if(node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboAttribute* href;
    if(node->v.element.tag == GUMBO_TAG_A && (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
    {
        results.links.push_back(href->value);
    }
}

void analyseNode(AnalyseResults& results, GumboNode* node)
{
    search_for_links(results, node);
    search_for_words(results, node);

    if(node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboVector* children = &node->v.element.children;
    for(unsigned int i = 0; i < children->length; ++i)
    {
        analyseNode(results, static_cast<GumboNode*>(children->data[i]));
    }
}

AnalyseResults analyseResource(WebRessource const& res)
{
    AnalyseResults results;

    GumboOutput* output = gumbo_parse(res.data.c_str());

    AT_SCOPE_EXIT([output]{
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    });

    analyseNode(results, output->root);

    return results;
}
