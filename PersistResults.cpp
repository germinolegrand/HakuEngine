#include "PersistResults.h"

#include <pqxx>
#include <iostream>

template<class iterator, class F>
void foreach_append_separated(std::string& str, iterator first, iterator last, F&& f, std::string const& separator)
{
    for(auto it = first; it != last;)
    {
        str += f(*it);

        if(++it != last)
            str += separator;
    }
}

DatabaseSession::DatabaseSession():
    m_connection("user=postgres password=toor")
{}

void DatabaseSession::persist(ToBeCrawled const& cron, AnalyseResults const& analyse)
{
    pqxx::work w(m_connection);

    pqxx::result r0 = w.exec(
    "UPDATE crontab"
        " SET"
            " last_date=transaction_timestamp(),"
            " crawl_interval=" + w.quote(cron.between.count()) + ","
            " crawl_count=crawl_count + 1"
        " WHERE url=" + w.quote(cron.url)
    );

    if(!analyse.links.empty())
    {
        std::string query1 = "WITH new_values (url, crawl_interval) as ("
        " VALUES";

        foreach_append_separated(query1, analyse.links.begin(), analyse.links.end(),
        [&w, &cron](decltype(analyse.links)::value_type const& link){
            return
            " ("
                " " + w.quote(link) + ","
                " " + pqxx::to_string(cron.between.count()) +
            " )";
        }, ",");

        query1 +=
//        " ),"
//        " upsert as"
//        " ( "
//            " update crontab m "
//              " set crawl_interval = nv.crawl_interval"
//            " FROM new_values nv"
//            " WHERE m.url = nv.url"
//            " RETURNING m.*"
        " )"
        " INSERT INTO crontab (url, crawl_interval)"
        " SELECT url, crawl_interval"
        " FROM new_values"
        " WHERE NOT EXISTS (SELECT 1"
                          " FROM crontab up"
                          " WHERE up.url = new_values.url)";


    //    std::string query1 = "INSERT INTO crontab (url, crawl_interval) VALUES";
    //
    //    foreach_append_separated(query1, analyse.links.begin(), analyse.links.end(),
    //    [&w, &cron](decltype(analyse.links)::value_type const& link){
    //        return
    //        " ("
    //            " " + w.quote(link) + ","
    //            " " + w.quote(cron.between.count()) +
    //        " )";
    //    }, ",");

        pqxx::result r1 = w.exec(query1);
    }

    if(!analyse.words.empty())
    {
        std::string query2 = "INSERT INTO keywords (keyword, score, url) VALUES";

        foreach_append_separated(query2, analyse.words.begin(), analyse.words.end(),
        [&w, &cron](decltype(analyse.words)::value_type const& pair_word_nb){
            return
            " ("
                " " + w.quote(pair_word_nb.first) + ","
                " " + w.quote(pair_word_nb.second) + ","
                " " + w.quote(cron.url) +
            " )";
        }, ",");

        pqxx::result r2 = w.exec(query2);
    }

    w.commit();

    for(auto& link : analyse.links)
        std::cout << link << std::endl;

    for(auto& pair_word_nb : analyse.words)
        std::cout << pair_word_nb.first << ":" << pair_word_nb.second << std::endl;
}

void DatabaseSession::persist_redirected(URL const& old_url, ToBeCrawled const& cron, AnalyseResults const& analyse)
{
    ///TODO modifier tout ce qui est en rapport avec old_url

    persist(cron, analyse);
}

void DatabaseSession::persist_error(URL const& url)
{
    pqxx::work w(m_connection);

    pqxx::result d = w.exec(
    "DELETE FROM crontab"
        " WHERE url=" + w.quote(url)
    );

    ///TODO supprimer tout ce qui est en rapport avec cette ressource

    w.commit();
}

std::vector<ToBeCrawled> DatabaseSession::getCrontab()
{
    pqxx::work w(m_connection);
    pqxx::result r = w.exec(
    "SELECT url, last_date, crawl_interval"
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
        tbc.between = std::chrono::milliseconds(row["crawl_interval"].as<int>());

        tobecrawled.emplace_back(tbc);

        std::cout << tbc.url << "\t" << tbc.between.count() << "\t" << row["last_date"] << std::endl;
    }

    return tobecrawled;
}

void DatabaseSession::updateCrontab(std::vector<ToBeCrawled> const& crontab)
{
    pqxx::work w(m_connection);

    std::string query = "INSERT OR UPDATE INTO crontab (url, last_date, crawl_interval) VALUES ";

    for(auto const& row : crontab)
    {
        query += "(";
        query += w.quote(row.url);
        query += ", ";
        query += "now()";
        query += ", ";
        query += w.quote(row.between.count());
        query += "),";
    }

    pqxx::result r = w.exec(query);

    w.commit();
}


