#include "SearchServer.h"
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <cstdlib>


std::mutex db_lock_search; // синхронизация потоков

void SearchServer::countWordCountInTread(std::vector<WordCount>& unique_pull, std::vector<Entry> count, int iterator)
{
    int sum= 0;
    for (auto iter: count)
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
    for (int request_index = 0; request_index <= queries_input.size(); ++request_index)
    {
        // Создание уникального массива с общим количеством слов в тексте
        doc_to_answer.emplace_back();
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
                int iterator_unique_pull = unique_pull.size();
                unique_pull.emplace_back(buffer_wordcount);
                std::vector<Entry> buffer_count = _index.getWordCount(buffer_wordcount.word);
                count_word_in_bd.emplace_back([&unique_pull, buffer_count, iterator_unique_pull, this]()
                {
                    countWordCountInTread(unique_pull, buffer_count, iterator_unique_pull);
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
            std::vector<Entry> buffer_docid = _index.getWordCount(unique_pull[iter_unique_word].word);
            for (int iter_word_to_search = iter_unique_word + 1;
                     iter_word_to_search < unique_pull.size();
                     ++iter_word_to_search)
            {
                std::vector<Entry> buffer_word_to_search = _index.getWordCount(unique_pull[iter_word_to_search].word);
                for (auto docs: buffer_docid)
                {
                    bool word_in_doc = false;
                    for (auto docs_to_search: buffer_word_to_search)
                        if (docs.doc_id == docs_to_search.doc_id)
                            word_in_doc = true;
                    if (!word_in_doc) docs.count = 0;
                }
            }
            for (auto docs: buffer_docid)
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
                doc_to_answer.emplace_back(buffer_doc_out);
                break;
            }
        }
        // Расчет релевантности для полученных документов.
        if (doc_to_answer.back().empty()) continue;
        std::vector<RelativeIndex> relevant;
        float max = 0;
        for (auto doc:doc_to_answer.back())
        {
            for (auto word: unique_pull)
            {
                for (auto word_docs: _index.getWordCount(word.word))
                {
                    if (doc.doc_id == word_docs.doc_id)
                    {
                        bool exist = false;
                        for (auto rel_index: relevant)
                        {
                            if (rel_index.doc_id == word_docs.doc_id)
                            {
                                rel_index.rank += word_docs.count;
                                exist = true;
                                if (max < rel_index.rank)
                                    max = rel_index.rank;
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
        for (auto doc: relevant)
        {
            doc.rank = doc.rank/max;
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
    }
    if (doc_to_answer.empty()) return {};
    return doc_to_answer;
}