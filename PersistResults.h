#ifndef PERSISTRESULTS_H
#define PERSISTRESULTS_H

#include "AnalyseResource.h"

#include <chrono>
#include <pqxx/pqxx>

struct ToBeCrawled
{
    URL url;
    int crawl_interval;
    int ttl;
};

class DatabaseSession
{
public:
    DatabaseSession(std::string const& connection_string = "");
    DatabaseSession(DatabaseSession&&) = delete;

    void persist(ToBeCrawled const& cron, AnalyseResults const& analyse);
    void persist_redirected(URL const& old_url, ToBeCrawled const& cron, AnalyseResults const& analyse);
    void persist_error(URL const& url);

    std::vector<ToBeCrawled> getCrontab();

private:
    pqxx::connection m_connection;
};

#endif // PERSISTRESULTS_H
