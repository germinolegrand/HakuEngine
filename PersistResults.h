#ifndef PERSISTRESULTS_H
#define PERSISTRESULTS_H

#include "AnalyseResource.h"

#include <chrono>
#include <pqxx>

struct ToBeCrawled
{
    unsigned int id;
    URL url;
    std::chrono::milliseconds between;
    std::chrono::milliseconds last;
};

class DatabaseSession
{
public:
    DatabaseSession();
    DatabaseSession(DatabaseSession&&) = delete;

    void persist(AnalyseResults const& analyse);

    std::vector<ToBeCrawled> getCrontab();

private:
    pqxx::connection m_connection;
};

#endif // PERSISTRESULTS_H
