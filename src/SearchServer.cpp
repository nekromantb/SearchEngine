#include "SearchServer.h"
#include "ConverterJSON.h"
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>


std::mutex db_lock_search; // синхронизация потоков

void SearchServer::countWordCountInTread(std::vector<WordCount>& unique_pull, int iterator)
{
    int sum= 0;
    for (auto & iter: unique_pull[iterator].doc_count)
    {
        sum += (int)iter.count;
    }
    db_lock_search.lock();
    unique_pull[iterator].count = sum;
    db_lock_search.unlock();
}

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string> &queries_input,
                                                             const int response_limit)
{
    std::vector<std::vector<RelativeIndex>> doc_to_answer;
    for (auto & request_index : queries_input)
    {
        // Создание уникального массива с общим количеством слов в тексте
        std::vector<WordCount> unique_pull;
        std::stringstream buffer_request(request_index);
        std::vector<std::thread> count_word_in_bd;
        while (!buffer_request.eof())
        {
            std::string buffer_word;
            buffer_request >> buffer_word;
            bool unique = true;
            for (auto & iter_pull : unique_pull)
                if (iter_pull.word == buffer_word)
                    unique = false;
            if (unique)
            {
                WordCount buffer_wordcount;
                buffer_wordcount.word = buffer_word;
                buffer_wordcount.doc_count = _index.getWordCount(buffer_wordcount.word);
                int iterator_unique_pull = (int)unique_pull.size();
                unique_pull.emplace_back(buffer_wordcount);
                count_word_in_bd.emplace_back([&unique_pull, iterator_unique_pull, this]()
                {
                    countWordCountInTread(unique_pull, iterator_unique_pull);
                });
            }
        }
        for (int iter = 0; iter<unique_pull.size();++iter)
        {
            count_word_in_bd.at(iter).join();
        }
        // Сортировка по возрастанию count массива уникальных слов
        std::sort(unique_pull.begin(),
                   unique_pull.end(),
                   [](const WordCount& x, const WordCount& y)->bool
                  {
                        return x.count < y.count;
                  });
        // Зацикленный отбор документов. закончится при достижении лимита на выдачу или если документов не останется.
        std::vector<RelativeIndex> buffer_doc_out;
        for (int iter_unique_word = 0; iter_unique_word < unique_pull.size(); ++iter_unique_word)
        {
            for (int iter_word_to_search = iter_unique_word + 1;
                     iter_word_to_search < unique_pull.size();
                     ++iter_word_to_search)
            {
                for (auto & docs: unique_pull[iter_unique_word].doc_count)
                {
                    bool word_in_doc = false;
                    for (auto & docs_to_search: unique_pull[iter_word_to_search].doc_count)
                        if (docs.doc_id == docs_to_search.doc_id)
                            word_in_doc = true;
                    if (!word_in_doc) docs.count = 0;
                }
            }
            for (auto & docs: unique_pull[iter_unique_word].doc_count)
            {
                if (docs.count!=0)
                {
                    bool exist = false;
                    for (auto & docs_in_answer: buffer_doc_out)
                        if(docs.doc_id ==docs_in_answer.doc_id)
                            exist = true;
                    if(!exist)
                    {
                        RelativeIndex buffer_one_doc_out;
                        buffer_one_doc_out.doc_id = docs.doc_id;
                        buffer_doc_out.emplace_back(buffer_one_doc_out);
                    }
                }
            }
            if (buffer_doc_out.size() >= response_limit)
            {
                break;
            }
        }
        if (buffer_doc_out.empty())
        {
            doc_to_answer.emplace_back();
            continue;
        }
        // Расчет релевантности для полученных документов.
        std::vector<RelativeIndex> relevant;
        float max = 0;
        for (auto & doc:buffer_doc_out)
        {
            for (auto & word_in_pull: unique_pull)
            {
                word_in_pull.doc_count = _index.getWordCount(word_in_pull.word);
                for (auto & word_docs: word_in_pull.doc_count)
                {
                    if (doc.doc_id == word_docs.doc_id)
                    {
                        bool exist = false;
                        for (auto & rel_index : relevant)
                        {
                            if (rel_index.doc_id == word_docs.doc_id)
                            {
                                rel_index.rank += (float)word_docs.count;
                                exist = true;
                                if (max < rel_index.rank)
                                    max = rel_index.rank;
                            }
                        }
                        if (!exist)
                        {
                            RelativeIndex buffer;
                            buffer.doc_id = word_docs.doc_id;
                            buffer.rank = (float)word_docs.count;
                            relevant.emplace_back(buffer);
                            if (max < relevant.back().rank)
                                max = relevant.back().rank;
                        }
                    }
                }
            }
        }
        for (auto & doc : relevant)
        {
            doc.rank = roundf(doc.rank) / roundf(max);
        }
        // Сортировка релевантности по убыванию.
        std::sort(relevant.begin(),
                  relevant.end(),
                  [](const RelativeIndex& x, const RelativeIndex& y)->bool
                  {
                      return x.rank > y.rank || ((x.rank == y.rank) && (x.doc_id < y.doc_id));
                  });
        // Добавление результата в результирующий вектор
        if (!relevant.empty())
            doc_to_answer.emplace_back(relevant);
    }
    if (doc_to_answer.empty()) return {};
    for (auto & answer_for_response : doc_to_answer)
        if (answer_for_response.size() > response_limit)
            answer_for_response.resize(response_limit);
    return doc_to_answer;
}