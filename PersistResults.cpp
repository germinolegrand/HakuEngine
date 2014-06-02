#include "PersistResults.h"

#include <pqxx>
#include <iostream>

DatabaseSession::DatabaseSession():
    m_connection("dbname=table_search user=postgres password=toor")
{
    pqxx::work w(m_connection);
    pqxx::result r = w.exec("SELECT 1+1");
    w.commit();

    std::cout << r[0][0].as<int>() << std::endl;
}

void DatabaseSession::persist(AnalyseResults const& analyse)
{
    for(auto& link : analyse.links)
        std::cout << link << std::endl;

    for(auto& pair_word_nb : analyse.words)
        std::cout << pair_word_nb.first << ":" << pair_word_nb.second << std::endl;
}

std::vector<ToBeCrawled> DatabaseSession::getCrontab()
{
    std::vector<ToBeCrawled> tobecrawled;
    return tobecrawled;
}
