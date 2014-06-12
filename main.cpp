#include <iostream>
#include <system_error>
#include <thread>

#include "DownloadResource.h"
#include "AnalyseResource.h"
#include "PersistResults.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello Miyazaki's world!" << std::endl;

    DatabaseSession dbsession(argc >= 2 ? argv[1] : "user=postgres");

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

                AnalyseResults results = analyseResource(webres);

                std::cout << results.full_text << std::endl;

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
                std::cerr << "\n" << tobecrawled.url << " : Unknown protocol " << e.protocol << std::endl;
                dbsession.persist_error(tobecrawled.url);
            }
            catch(ImpossibleAccess const& e)
            {
                std::cerr << "\n" << tobecrawled.url << " : Can't access " << e.uri << " with error code " << e.error_code << std::endl;
                dbsession.persist_error(tobecrawled.url);
            }
            catch(UnknownContentType const& e)
            {
                std::cerr << "\n" << tobecrawled.url << " : Unknown content type " << e.content_type << std::endl;
                dbsession.persist_error(tobecrawled.url);
            }
            catch(pqxx::sql_error const&e)
            {
                std::cerr << "\n" << tobecrawled.url << e.what() << std::endl;
            }
        }
    }

    return 0;
}
