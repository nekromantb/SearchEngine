#pragma once

#include "nlohmann/json.hpp"
using json = nlohmann::json;

class ConverterJSON
{
    bool readyForWork = false; // флаг готовности к работе с БД
    bool forceExit = false; // флаг отказа пользователя от работы с программой (не прочитаны конфиг фаилы)
    json config;
    json requests;
    json answers;
    std::string configPath = "config.json";
    std::string requestsPath = "requests.json";
    std::string answersPath = "/output/answers.json";
    /**
    * Методы изменения пути к конфигурационным фаилам.
    * Выставляет флаг readyForWork в false.
    * Выводит в консоль текущую конфигурацию программы.
    */
    void setConfigPath();
    void setRequestsPath();
public:
    /**
     * Конструктор класса не используется для возможности проверки
     * и изменения конфигурационных фаилов
     */
    ConverterJSON() = default;

    /**
     * Метод загрузки данных из фаилов (однократное чтение данных)
     * Запись идет во внутренние поля класса.
     * Проводит проверку валидности конфигурационных фаилов.
     * В случае ошибки валидности предложит изменить путь к конфигурационным фаилам.
     * Выставляет флаг готовности к работе readyForWork.
     */
    void readConfigJsonFiles();

    /**
    * Метод получения содержимого файлов
    * @return Возвращает список с содержимым файлов перечисленных
    * в config.json
    */
    std::vector<std::string> getTextDocuments();

    /**
    * Метод считывает поле max_responses для определения предельного
    * количества ответов на один запрос
    * @return
    */
    int getResponsesLimit();

    /**
    * Метод получения запросов из файла requests.json
    * @return возвращает список запросов из файла requests.json
    */
    std::vector<std::string> getRequests();

    /**
    * Положить в файл answers.json результаты поисковых запросов
    */
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);

    /**
     * Метод проверки готовности движка к работе.
     * @return возвращает флаг готовности readyForWork
     */
    [[nodiscard]] bool getStatus() const;
};