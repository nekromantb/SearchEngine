#pragma once
#include <string>
#include <vector>
#include <map>


struct Entry
{
    size_t doc_id, count;

    // Данный оператор необходим для проведения тестовых сценариев
    bool operator ==(const Entry& other) const
    {
        return (doc_id == other.doc_id &&
                count == other.count);
    }
};

class InvertedIndex
{
    /** Решил не хранить тексты документов. Памяти занимают много,
    * а в программе используются однократно при заполнении индекса.
    * Необходимую информацию в виде названия документа можно взять из конфигурационного номера по docid
    */
    std::map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь
public:
    /**
     * Конструктор класса не используется
     */
    InvertedIndex() = default;
    /**
    * Обновить или заполнить базу документов, по которой будем совершать
    поиск. При чтении происходит индексирование и обновление внутренней базы индексов.*
    @param texts_input содержимое документов
    * Метод удаляет старую БД!
    */
    void updateDocumentBase(std::vector<std::string> input_docs);
    /**
    * Метод определяет количество вхождений слова word в загруженной базе
    документов. Используется в поисковом движке для обращения к БД, хранящейся в классе.
    * @param word слово, частоту вхождений которого необходимо определить
    * @return возвращает подготовленный список с частотой слов из freq_dictionary
    */
    std::vector<Entry> getWordCount(const std::string& word);
private:
    /**
     * Вспомогательный метод для многопотточности.
     * @param docid_iter принимает на вход docid текущего отбрабатываемого фаила
     * @param input_string содержимое документа
     */
    void oneThreadUpdateDocumentBase(int docid_iter, const std::string& input_string);
};
