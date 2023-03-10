#pragma once
#include <string>
#include <vector>
#include "InvertedIndex.h"

struct RelativeIndex{
    size_t doc_id = 0;
    float rank = 0;
    bool operator ==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

struct WordCount
{
    std::string word;
    int count = 0;
    std::vector<Entry> doc_count = {};
};

class SearchServer
{
    InvertedIndex _index;
    /**
     * Внутренний метод подсчета общего количества вхождений слов из запроса
     * в заданные тексты для проведения сортировки (проводится в многопоточном режиме)
     * @param word структура, хранящяя слово и количество его вхождений
     * @param iterator итератор текущего слова.
     */
    void countWordCountInTread(std::vector<WordCount>& word, int iterator);
public:
    /**
    * @param idx в конструктор класса передаётся ссылка на класс
    InvertedIndex,
    * чтобы SearchServer мог узнать частоту слов встречаемых в
    запросе
    */
    SearchServer(InvertedIndex &idx) : _index(idx) {};

    /**
    * Метод обработки поисковых запросов
    * @param queries_input поисковые запросы взятые из файла
    requests.json
    * @param responce_limit лимит выдачи документов
    * @return возвращает отсортированный список релевантных ответов для
    заданных запросов
    */
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string> &queries_input,
                                                   int response_limit);
};
