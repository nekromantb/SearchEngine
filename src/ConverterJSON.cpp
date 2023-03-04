#include "ConverterJSON.h"
#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <cstdio>
#include "nlohmann/json.hpp"
#include "SearchServer.h"
using json = nlohmann::json;

class ConfigurationFileIsEmptyException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "Config file is empty!";
    }
};

class ConfigurationFileIsMissingException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "Config file is missing!";
    }
};

class RequestsFileIsEmptyException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "Requests file is empty!";
    }
};

class RequestsFileIsMissingException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "Requests file is missing!";
    }
};

class ConfigVersionIsWrongException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "config.json has incorrect file version";
    }
};

class ConfigNoDataFilesException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "config.json has no data inside";
    }
};

class ConfigNoMaxResponseException: public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "config.json has no max_response data";
    }
};


void ConverterJSON::readConfigJsonFiles()
{

    while (!readyForWork)
    {
        /** working with config.json */
        if (forceExit) break;
        config.clear();
        try
        {
            std::ifstream configFile(configPath);
            if (!configFile.is_open()) throw ConfigurationFileIsMissingException();
            configFile >> config;
            configFile.close();
            if (!config.contains("config") || !config["config"].contains("name"))
                throw ConfigurationFileIsEmptyException();
            if (!config["config"].contains("version") || config["config"]["version"] != 0.1)
                throw ConfigVersionIsWrongException();
            if (!config.contains("files") || config["files"].empty())
                throw ConfigNoDataFilesException();
            readyForWork = true;
        }
        catch(const ConfigurationFileIsEmptyException& x)
        {
            std::cerr << "Caught exception: " << x.what() <<std::endl;
            setConfigPath();
        }
        catch(const ConfigurationFileIsMissingException& x)
        {
            std::cerr << "Caught exception: " << x.what() <<std::endl;
            setConfigPath();
        }
        catch(const ConfigVersionIsWrongException& x)
        {
            std::cerr << "Caught exception: " << x.what() <<std::endl;
            setConfigPath();
        }
        catch (const ConfigNoDataFilesException &x)
        {
            std::cerr << "Caught exception: " << x.what() << std::endl;
            setConfigPath();
        }


        /** working with requests.json */
        if (forceExit) break;
        requests.clear();
        try
        {
            std::ifstream requestsFile(requestsPath);
            if (!requestsFile.is_open()) throw RequestsFileIsMissingException();
            requestsFile >> requests;
            requestsFile.close();
            if (!requests.contains("requests") || requests["requests"].empty())
                throw RequestsFileIsEmptyException();
            readyForWork = true;
        }
        catch(const RequestsFileIsEmptyException& x)
        {
            std::cerr << "Caught exception: " << x.what() <<std::endl;
            setRequestsPath();
        }
        catch(const RequestsFileIsMissingException& x)
        {
            std::cerr << "Caught exception: " << x.what() <<std::endl;
            setRequestsPath();
        }
    }
    std::cout << config["config"]["name"] << " version " << config["config"]["version"] << std::endl;
}

void ConverterJSON::setConfigPath()
{
    std::cout << "You can change path to config.json file, to do this type \"c\""<<std::endl;
    std::cout << "To exit from program type \"e\""<<std::endl;
    std::cout << "You can change files and try again, to do this type \"r\""<<std::endl;
    std::string inputCommand;
    std::cin >> inputCommand;
    if (inputCommand == "c" || inputCommand == "C")
    {
        readyForWork = false;

        /** Configuration Path changing */
        std::cout << "Changing configuration path initialised."<< std::endl;
        std::cout << "Enter config.json file path:"<< std::endl;
        std::cin >> configPath;

        /** Configuration path output in console */
        std::cout << "Configuration file path changed!" << std::endl;
        std::cout << "Configuration file: " << configPath << std::endl;
    }
    if (inputCommand == "e" || inputCommand == "E")
    {
        readyForWork = false;
        forceExit = true;
    }
    if (inputCommand == "r" || inputCommand == "R")
    {
        readyForWork = false;
    }
}

void ConverterJSON::setRequestsPath()
{
    std::cout << "You can change path to request.json file, to do this type \"c\""<<std::endl;
    std::cout << "To exit from program type \"e\""<<std::endl;
    std::cout << "To try again type \"r\""<<std::endl;
    std::string inputCommand;
    std::cin >> inputCommand;
    if (inputCommand == "c" || inputCommand == "C")
    {
        readyForWork = false;

        /** Request path changing */
        std::cout << "Changing requests path initialised."<< std::endl;
        std::cout << "Enter request.json file path:"<< std::endl;
        std::cin >> requestsPath;

        /** Request path output in console */
        std::cout << "Requests file path changed!" << std::endl;
        std::cout << "Request file: " << requestsPath << std::endl;
    }
    if (inputCommand == "e" || inputCommand == "E")
    {
        readyForWork = false;
        forceExit = true;
    }
    if (inputCommand == "r" || inputCommand == "R") {
        readyForWork = false;
    }
}

std::vector<std::string> ConverterJSON::getTextDocuments()
{
    std::vector<std::string> docData;
    for (std::string i: config["files"]) {
        std::string buffer_word, buffer_string;
        std::ifstream inputData(i);
        while (!inputData.eof())
        {
            inputData >> buffer_word;
            buffer_string += (buffer_word + " ");
        }
        docData.push_back(buffer_string);
        inputData.close();
    }
    return docData;
}

int ConverterJSON::getResponsesLimit()
{
    int limit;
    try
    {
        if (!config["config"].contains("max_responses") || config["config"]["max_responses"].empty())
            throw ConfigNoMaxResponseException();
        limit = config["config"]["max_responses"];
    }
    catch (const ConfigNoMaxResponseException& x)
    {
        std::cerr << "Caught exception: " << x.what() <<std::endl;
        std::cout << "Max Response set to 5" <<std::endl;
        config["config"]["max_responses"] = 5;
        limit = config["config"]["max_responses"];
    }
    return limit;
}

std::vector<std::string> ConverterJSON::getRequests()
{
    std::vector<std::string> requestsData;
    for (std::string request: requests["requests"])
    {
        requestsData.push_back(request);
    }
    return requestsData;
}

void ConverterJSON::putAnswers(std::vector<std::vector<RelativeIndex>> answersOutput)
{
    /** Filling answers map*/
    for (int requestsIterator = 0; requestsIterator < answersOutput.size(); requestsIterator++ )
    {
        char numberRequest[11];
        std::sprintf(numberRequest, "request %03i", requestsIterator);
        if (answersOutput[requestsIterator].empty())
        {
            answers["answers"][std::string(numberRequest)]["result"] = "false";
        }
        else
        {
            if (answersOutput[requestsIterator].size() == 1)
            {
                answers["answers"] =
                answers["answers"][std::string(numberRequest)]["result"] =
                        "true";
                answers["answers"][std::string(numberRequest)]["docid"] =
                        answersOutput[requestsIterator][0].doc_id;
                answers["answers"][std::string(numberRequest)]["rank"] =
                        answersOutput[requestsIterator][0].rank;
            }
            else{
                answers["answers"][std::string(numberRequest)]["result"] = "true";
                for (int iter_answers = 0;
                iter_answers < answersOutput[requestsIterator].size();
                ++iter_answers)
                {
                    answers["answers"][std::string(numberRequest)]["relevance"].push_back({
                        {"docid",answersOutput[requestsIterator][iter_answers].doc_id},
                        {"rank",answersOutput[requestsIterator][iter_answers].rank}
                    });
                }
            }
        }
    }
    /** Writing data to file (Rewriting. If not exists will create new one.) */
    std::ofstream answersFile (answersPath);
    answersFile << answers;
    answersFile.close();
}

bool ConverterJSON::getStatus() const
{
    return readyForWork;
}