#include <iostream>
#include <system_error>
#include <thread>

#include "DownloadResource.h"
#include "AnalyseResource.h"
#include "PersistResults.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello Miyazaki's world!" << std::endl;

//    std::string resource = downloadURI(uri);
//    AnalyseResults analyseResults = analyseResource(uri, resource);
//    persistAnalyseResults(uri, analyseResults);

    //WebRessource indexHaku = downloadFromURL("http://192.168.1.36/");

    DatabaseSession dbsession;

    while(true)
    {
        std::vector<ToBeCrawled> crontab = dbsession.getCrontab();

        if(crontab.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(30000));
            continue;
        }

        for(auto& tobecrawled : crontab)
        {
            try
            {
                WebRessource webres = downloadFromURL(tobecrawled.url);

                //std::cout << webres.data << std::endl;

                AnalyseResults results = analyseResource(webres);

                if(tobecrawled.url == webres.url)
                {
                    dbsession.persist(tobecrawled, results);
                }
                else
                {
                    std::swap(webres.url, tobecrawled.url);
                    dbsession.persist_redirected(webres.url, tobecrawled, results);
                }
            }
            catch(UnknownProtocol const& e)
            {
                std::cerr << "Unknown protocol " << e.protocol << std::endl;
                dbsession.persist_error(tobecrawled.url);
            }
            catch(ImpossibleAccess const& e)
            {
                std::cerr << "Can't access " << e.uri << " with error code " << e.error_code << std::endl;
                dbsession.persist_error(tobecrawled.url);
            }
        }
    }

    return 0;
}
