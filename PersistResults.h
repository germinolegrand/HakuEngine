#ifndef PERSISTRESULTS_H
#define PERSISTRESULTS_H

#include "AnalyseResource.h"

#include <chrono>
#include <pqxx>

struct ToBeCrawled
{
    URL url;
    std::chrono::milliseconds between;
};

class DatabaseSession
{
public:
    DatabaseSession();
    DatabaseSession(DatabaseSession&&) = delete;

    void persist(ToBeCrawled const& cron, AnalyseResults const& analyse);
    void persist_redirected(URL const& old_url, ToBeCrawled const& cron, AnalyseResults const& analyse);
    void persist_error(URL const& url);

    std::vector<ToBeCrawled> getCrontab();
    void updateCrontab(std::vector<ToBeCrawled> const& crontab);

private:
    pqxx::connection m_connection;
};

#endif // PERSISTRESULTS_H
