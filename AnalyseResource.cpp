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
    if(word.size() >= 1 && std::string(",.").find(word[word.size() - 1]) != std::string::npos)
        word.resize(word.size() - 1);

    if(word.size() < 3)
        throw SkipThisWord();

    return word;
}

void search_for_img(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == GUMBO_TAG_IMG)
    {
        GumboAttribute* src;
        GumboAttribute* alt;

        if((src = gumbo_get_attribute(&node->v.element.attributes, "src"))
           && (alt = gumbo_get_attribute(&node->v.element.attributes, "alt")))
        {
            URL url = link_to_full_url(webres.url, src->value);
            results.links.insert(url);

            std::string text(alt->value);

            for(auto beg = begin(text),
                      en = end(text),
                      it = begin(text)
                ; it != en ;)
            {
                beg = std::find_if(it, en, utils::isgraph);
                it = std::find_if(beg, en, utils::isnotgraph);

                try
                {
                    results.backlinks.emplace_back(normalize_word({beg, it}), url);
                }
                catch(SkipThisWord const& e)
                {}
            }
        }
    }
}

std::function<void(WebRessource const& webres, AnalyseResults& results, GumboNode* node)>
generate_skip_tag(GumboTag tag)
{
    return [tag](WebRessource const& webres, AnalyseResults& results, GumboNode* node){
        if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == tag)
            throw SkipThisNode{};
    };
}

std::function<bool(WebRessource const& webres, AnalyseResults& results, GumboNode* node)>
generate_is_inside_tag(GumboTag tag)
{
    GumboNode* strong_node = nullptr;
    unsigned int strong_node_children_countdown = 0;

    return [tag, strong_node, strong_node_children_countdown](WebRessource const& webres, AnalyseResults& results, GumboNode* node) mutable {
        if(node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == tag)
        {
            strong_node_children_countdown = node->v.element.children.length;

            if(strong_node_children_countdown > 0)
                strong_node = node;

            return false;
        }

        bool ret = strong_node != nullptr;

        if(strong_node && node->parent == strong_node)
        {
            --strong_node_children_countdown;

            if(strong_node_children_countdown == 0)
                strong_node = nullptr;
        }

        return ret;
    };
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
        beg = std::find_if(it, en, utils::isgraph);
        it = std::find_if(beg, en, utils::isnotgraph);

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

void analyse_node(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
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
            analyse_node(webres, results, static_cast<GumboNode*>(children->data[i]));
        }
        catch(SkipThisNode const&){}
    }
}

AnalyseResults analyse_text_html(WebRessource const& webres)
{
    AnalyseResults results;

    GumboOutput* output = gumbo_parse(webres.data.c_str());

    AT_SCOPE_EXIT([output]{
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    });

    analyse_node(webres, results, output->root);

    return results;
}

AnalyseResults analyse_ftp_file(WebRessource const& webres)
{
    AnalyseResults results;

    unsigned int score = 1;

    URI uri = uri_of_url(webres.url);

    for(auto it = begin(uri); it != end(uri);)
    {
        auto pair_beg_en = extract_between(it, end(uri), '/', '/');

        ///Insert or add
        {
            auto pair_it_bool = results.words.emplace(std::string{pair_beg_en.first, pair_beg_en.second}, score);

            if(!pair_it_bool.second)
                pair_it_bool.first->second += score;
        }

        it = pair_beg_en.second;
    }

    return results;
}

AnalyseResults Analyzer::operator()(WebRessource const& webres)
{
    try
    {
        return m_analysers.at(webres.content_type)(webres);
    }
    catch(std::out_of_range const&)
    {
        throw UnknownContentType{webres.content_type};
    }
}

void Analyzer::setAnalyser(ContentType const& content_type, std::function<AnalyseResults(WebRessource const&)> analyser)
{
    m_analysers[content_type] = std::move(analyser);
}

AnalyseResults TextHTMLAnalyser::operator()(WebRessource const& webres)
{
    AnalyseResults results;

    GumboOutput* output = gumbo_parse(webres.data.c_str());

    AT_SCOPE_EXIT([output]{
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    });

    analyse_node(webres, results, output->root);

    return results;
}

void TextHTMLAnalyser::addAnalyser(HTMLAnalyser analyser)
{
    m_analysers.push_back(std::move(analyser));
}

void TextHTMLAnalyser::analyse_node(WebRessource const& webres, AnalyseResults& results, GumboNode* node)
{
    for(auto& analyser : m_analysers)
        analyser(webres, results, node);

    if(node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboVector* children = &node->v.element.children;
    for(unsigned int i = 0; i < children->length; ++i)
    {
        try
        {
            analyse_node(webres, results, static_cast<GumboNode*>(children->data[i]));
        }
        catch(SkipThisNode const&){}
    }
}


