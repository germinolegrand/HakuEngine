#include <iostream>
#include <system_error>
#include "tinyxml2/tinyxml2.h"

#include "DownloadResource.h"
#include "AnalyseResource.h"
#include "PersistResults.h"

struct PageVisitor: public tinyxml2::XMLVisitor
{
    bool Visit(tinyxml2::XMLText const& text) override
    {
        std::cout << text.Value() << std::endl;
        return true;
    }
};

std::error_category& XMLError_category()
{
    struct category: public std::error_category
    {
        const char* name() const noexcept override
        { return "XMLError:"; }

        std::string message(int e) const noexcept override
        {
            using namespace tinyxml2;
            switch(e)
            {
                case XML_NO_ERROR:                          return "No error";
                case XML_NO_ATTRIBUTE:                      return "No attribute";
                case XML_WRONG_ATTRIBUTE_TYPE:              return "Wrong attribute type";
                case XML_ERROR_FILE_NOT_FOUND:              return "File not found";
                case XML_ERROR_FILE_COULD_NOT_BE_OPENED:    return "File could not be opened";
                case XML_ERROR_FILE_READ_ERROR:             return "File read error";
                case XML_ERROR_ELEMENT_MISMATCH:            return "Element mismatch";
                case XML_ERROR_PARSING_ELEMENT:             return "Error parsing element";
                case XML_ERROR_PARSING_ATTRIBUTE:           return "Error parsing attribute";
                case XML_ERROR_IDENTIFYING_TAG:             return "Error identifying tag";
                case XML_ERROR_PARSING_TEXT:                return "Error parsing text";
                case XML_ERROR_PARSING_CDATA:               return "Error parsing CDATA";
                case XML_ERROR_PARSING_COMMENT:             return "Error parsing comment";
                case XML_ERROR_PARSING_DECLARATION:         return "Error parsing declaration";
                case XML_ERROR_PARSING_UNKNOWN:             return "Error parsing unknown";
                case XML_ERROR_EMPTY_DOCUMENT:              return "Empty document";
                case XML_ERROR_MISMATCHED_ELEMENT:          return "Mismatched element";
                case XML_ERROR_PARSING:                     return "Error parsing";
                case XML_CAN_NOT_CONVERT_TEXT:              return "Can not convert text";
                case XML_NO_TEXT_NODE:                      return "No text node";
                default:    return "";
            }
        }
    };

    static category _cat;

    return _cat;
}


int main(int argc, char* argv[])
{
    std::cout << "Hello Miyazaki's world!" << std::endl;

//    std::string resource = downloadURI(uri);
//    AnalyseResults analyseResults = analyseResource(uri, resource);
//    persistAnalyseResults(uri, analyseResults);

    //WebRessource indexHaku = downloadFromURL("http://192.168.1.36/");

    DatabaseSession dbsession;

    try
    {
        WebRessource indexHaku = downloadFromURL("http://www.glfw.org/");

        std::cout << indexHaku.data << std::endl;

        AnalyseResults results = analyseResource(indexHaku);

        dbsession.persist(results);
    }
    catch(ImpossibleAccess const& e)
    {
        std::cerr << "Can't access " << e.uri << " with error code " << e.error_code << std::endl;
    }

    return 0;
}
