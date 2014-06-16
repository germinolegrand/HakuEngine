#include "PersistResults.h"

#include "utils.h"

#include <pqxx/pqxx>
#include <iostream>

DatabaseSession::DatabaseSession(std::string const& connection_string):
    m_connection(connection_string)
{}

void DatabaseSession::persist(ToBeCrawled const& cron, AnalyseResults const& analyse)
{
    pqxx::work w(m_connection);

    pqxx::result r0 = w.exec(
    "UPDATE crontab"
        " SET"
            " last_date=transaction_timestamp(),"
            " crawl_interval=" + w.quote(cron.crawl_interval) + ","
            " crawl_count=crawl_count + 1"
        " WHERE url=" + w.quote(cron.url)
    );

    if(cron.ttl != 1 && !analyse.links.empty())
    {
        std::string query1 = "WITH new_values (url, crawl_interval, ttl) as ("
        " VALUES";

        foreach_append_separated(query1, analyse.links.begin(), analyse.links.end(),
        [&w, &cron](decltype(analyse.links)::value_type const& link){
            return
            " ("
                " " + w.quote(link) + ","
                " " + pqxx::to_string(cron.crawl_interval) + ","
                " " + pqxx::to_string(cron.ttl == 0 ? 0 : cron.ttl - 1) +
            " )";
        }, ",");

        query1 +=
        " ),"
        " upsert as"
        " ( "
            " update crontab m "
              " set crawl_interval = nv.crawl_interval"
            " FROM new_values nv"
            " WHERE m.url = nv.url"
            " RETURNING m.*"
        " )"
        " INSERT INTO crontab (url, crawl_interval, ttl)"
        " SELECT url, crawl_interval, ttl"
        " FROM new_values"
        " WHERE NOT EXISTS (SELECT 1"
                          " FROM upsert up"
                          " WHERE up.url = new_values.url)";

        pqxx::result r1 = w.exec(query1);
    }

    pqxx::result d2 = w.exec(
    "DELETE FROM keywords"
        " WHERE url=" + w.quote(cron.url)
    );

    if(!analyse.words.empty())
    {
        std::string query2 = "INSERT INTO keywords (keyword, score, url) VALUES";

        foreach_append_separated(query2, analyse.words.begin(), analyse.words.end(),
        [&w, &cron](decltype(analyse.words)::value_type const& pair_word_nb){
            return
            " ("
                " lower(" + w.quote(pair_word_nb.first) + "),"
                " " + w.quote(pair_word_nb.second) + ","
                " " + w.quote(cron.url) +
            " )";
        }, ",");

        pqxx::result r2 = w.exec(query2);
    }

    pqxx::result d3 = w.exec(
    "DELETE FROM backlinks"
        " WHERE backlink_url=" + w.quote(cron.url)
    );

    if(!analyse.backlinks.empty())
    {
        std::string query3 = "INSERT INTO backlinks (keyword, url, backlink_url) VALUES";

        foreach_append_separated(query3, analyse.backlinks.begin(), analyse.backlinks.end(),
        [&w, &cron](decltype(analyse.backlinks)::value_type const& pair_word_url){
            return
            " ("
                " lower(" + w.quote(pair_word_url.first) + "),"
                " " + w.quote(pair_word_url.second) + ","
                " " + w.quote(cron.url) +
            " )";
        }, ",");

        pqxx::result r3 = w.exec(query3);
    }

    w.commit();

//    for(auto& link : analyse.links)
//        std::cout << link << std::endl;
//
//    for(auto& pair_word_nb : analyse.words)
//        std::cout << pair_word_nb.first << ":" << pair_word_nb.second << std::endl;
}

void DatabaseSession::persist_redirected(URL const& old_url, ToBeCrawled const& cron, AnalyseResults const& analyse)
{
    ///TODO modifier tout ce qui est en rapport avec old_url

    persist(cron, analyse);
}

void DatabaseSession::persist_error(URL const& url)
{
    pqxx::work w(m_connection);

    pqxx::result d0 = w.exec(
    "DELETE FROM crontab"
        " WHERE url=" + w.quote(url)
    );

    pqxx::result d1 = w.exec(
    "DELETE FROM keywords"
        " WHERE url=" + w.quote(url)
    );

    ///TODO supprimer tout ce qui est en rapport avec cette ressource

    w.commit();
}

std::vector<ToBeCrawled> DatabaseSession::getCrontab()
{
    pqxx::work w(m_connection);
    pqxx::result r = w.exec(
    "SELECT url, last_date, crawl_interval, ttl"
        " FROM crontab"
        " WHERE transaction_timestamp() > last_date + crawl_interval * interval '1 hour'"
    );
    pqxx::result d = w.exec(
    "UPDATE crontab"
        " SET last_date=transaction_timestamp()"
        " WHERE transaction_timestamp() > last_date + crawl_interval * interval '1 hour'"
    );

    w.commit();

    std::vector<ToBeCrawled> tobecrawled;

    for(auto const& row : r)
    {
        ToBeCrawled tbc;
        tbc.url = row["url"].as<std::string>();
        tbc.crawl_interval = row["crawl_interval"].as<int>();
        tbc.ttl = row["ttl"].as<int>();

        tobecrawled.emplace_back(tbc);

        //std::cout << tbc.url << "\t" << tbc.crawl_interval << "\t" << row["last_date"] << "\t" << row["ttl"] << std::endl;
    }

    return tobecrawled;
}
