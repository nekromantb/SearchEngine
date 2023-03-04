#include "InvertedIndex.h"
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <thread>
#include <mutex>

std::mutex db_lock; // синхронизация потоков
void InvertedIndex::oneThreadUpdateDocumentBase(int docid_iter, std::string input_string) {
    std::stringstream buffer_string;
    buffer_string = std::stringstream (input_string);
    while (!buffer_string.eof())
    {
        std::string buffer_word;
        std::vector<Entry> buffer_vector;
        buffer_string >> buffer_word;
        db_lock.lock();
        auto dict_iterator = freq_dictionary.find(buffer_word);
        if (dict_iterator == freq_dictionary.end())
        {
            buffer_vector.push_back({size_t(docid_iter),1});
            freq_dictionary.insert(std::pair<std::string, std::vector<Entry>>(buffer_word, buffer_vector));
        }
        else
        {
            bool added = false;
            for (auto entry_doc: dict_iterator->second) {
                if (entry_doc.doc_id == docid_iter)
                {
                    ++dict_iterator->second[docid_iter].count;
                    added = true;
                }
            }
            if(added == false)
                dict_iterator->second.push_back({size_t(docid_iter), 1});
        }
        db_lock.unlock();
    }
}

void InvertedIndex::updateDocumentBase(std::vector<std::string> input_docs)
{
    freq_dictionary.clear();
    std::vector<std::thread> doc_update;
    for (int docid_iter = 0; docid_iter < input_docs.size(); ++docid_iter)
    {
        doc_update.emplace_back([docid_iter, this, input_docs](){
            oneThreadUpdateDocumentBase(docid_iter,input_docs[docid_iter]);
            });
    }
    for (int docid_iter = 0; docid_iter < input_docs.size(); ++docid_iter)
    {
        doc_update[docid_iter].join();
    }
}

std::vector<Entry> InvertedIndex::getWordCount(const std::string &word)
{
    auto iter = freq_dictionary.find(word);
    if ( iter == freq_dictionary.end())
        return {};
    else
        return iter->second;
}