#include "SearchServer.h"
#include "ConverterJSON.h"
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <cstdlib>


std::mutex db_lock_search; // синхронизация потоков

void SearchServer::countWordCountInTread(std::vector<WordCount>& unique_pull, int iterator)
{
    int sum= 0;
    for (auto iter: unique_pull[iterator].doc_count)
    {
        sum += iter.count;
    }
    db_lock_search.lock();
    unique_pull[iterator].count = sum;
    db_lock_search.unlock();
}

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string> &queries_input,
                                                             const int response_limit)
{
    std::vector<std::vector<RelativeIndex>> doc_to_answer;
    for (int request_index = 0; request_index < queries_input.size(); ++request_index)
    {
        // Создание уникального массива с общим количеством слов в тексте
        std::vector<WordCount> unique_pull;
        std::stringstream buffer_request(queries_input[request_index]);
        std::vector<std::thread> count_word_in_bd;
        while (!buffer_request.eof())
        {
            std::string buffer_word;
            buffer_request >> buffer_word;
            bool unique = true;
            for (int iter_pull = 0;
                     iter_pull<unique_pull.size();
                     ++iter_pull)
                if (unique_pull[iter_pull].word == buffer_word)
                    unique = false;
            if (unique)
            {
                WordCount buffer_wordcount;
                buffer_wordcount.word = buffer_word;
                buffer_wordcount.doc_count = _index.getWordCount(buffer_wordcount.word);
                int iterator_unique_pull = unique_pull.size();
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
        std::qsort(unique_pull.data(),
                   unique_pull.size(),
                   sizeof(decltype(unique_pull)::value_type),
                   [](const void* x, const void* y)
        {
            if ((((WordCount*)x)->count) < (((WordCount*)y)->count))
                return -1;
            if ((((WordCount*)x)->count) > (((WordCount*)y)->count))
                return 1;
            return 0;
        });
        // Зацикленный отбор документов. закончится при достижении лимита на выдачу или если документов не останется.
        std::vector<RelativeIndex> buffer_doc_out;
        for (int iter_unique_word = 0; iter_unique_word < unique_pull.size(); ++iter_unique_word)
        {
            for (int iter_word_to_search = iter_unique_word + 1;
                     iter_word_to_search < unique_pull.size();
                     ++iter_word_to_search)
            {
                for (auto docs: unique_pull[iter_unique_word].doc_count)
                {
                    bool word_in_doc = false;
                    for (auto docs_to_search: unique_pull[iter_word_to_search].doc_count)
                        if (docs.doc_id == docs_to_search.doc_id)
                            word_in_doc = true;
                    if (!word_in_doc) docs.count = 0;
                }
            }
            for (auto docs: unique_pull[iter_unique_word].doc_count)
            {
                if (docs.count!=0)
                {
                    bool exist = false;
                    for (auto docs_in_answer: buffer_doc_out)
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
            continue;
        }
        // Расчет релевантности для полученных документов.
        std::vector<RelativeIndex> relevant;
        double max = 0;
        for (auto doc:buffer_doc_out)
        {
            for (auto word: unique_pull)
            {
                //word.doc_count = _index.getWordCount(word.word);
                for (auto word_docs: word.doc_count)
                {
                    if (doc.doc_id == word_docs.doc_id)
                    {
                        bool exist = false;
                        for (int rel_index = 0; rel_index < relevant.size(); ++rel_index)
                        {
                            if (relevant[rel_index].doc_id == word_docs.doc_id)
                            {
                                relevant[rel_index].rank += word_docs.count;
                                exist = true;
                                if (max < relevant[rel_index].rank)
                                    max = relevant[rel_index].rank;
                            }
                        }
                        if (!exist)
                        {
                            RelativeIndex buffer;
                            buffer.doc_id = word_docs.doc_id;
                            buffer.rank = word_docs.count;
                            relevant.emplace_back(buffer);
                            if (max < relevant.back().rank)
                                max = relevant.back().rank;
                        }
                    }
                }
            }
        }
        for (int doc = 0; doc < relevant.size(); ++doc)
        {
            relevant[doc].rank = relevant[doc].rank / max;
        }
        // Сортировка релевантности по убыванию.
        std::qsort(relevant.data(),
                   relevant.size(),
                   sizeof(decltype(relevant)::value_type),
                   [](const void* x, const void* y)
                   {
                       if ((((RelativeIndex*)x)->rank) < (((RelativeIndex*)y)->rank))
                           return 1;
                       if ((((RelativeIndex*)x)->rank) > (((RelativeIndex*)y)->rank))
                           return -1;
                       return 0;
                   });
        // Добавление результата в результирующий вектор
        if (!relevant.empty())
            doc_to_answer.emplace_back(relevant);
        else
            doc_to_answer.emplace_back();
    }
    if (doc_to_answer.empty()) return {};
    return doc_to_answer;
}