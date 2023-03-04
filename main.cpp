#include <iostream>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"
#include <vector>
#include <string>

int main() {
    ConverterJSON files;
    files.readConfigJsonFiles();
    if (!files.getStatus())
        return 1;
    InvertedIndex idx;
    idx.updateDocumentBase(files.getTextDocuments());
    SearchServer srv(idx);
    std::vector<std::vector<RelativeIndex>> result = srv.search(files.getRequests(), files.getResponsesLimit());
    files.putAnswers(result);
    return 0;
}

//#include "gtest/gtest.h"
//
//int main() {
//    ::testing::InitGoogleTest();
//    return RUN_ALL_TESTS();
//}
//
//TEST(sample_test_case, sample_test)
//{
//    EXPECT_EQ(1, 1);
//}
//
//
//void TestInvertedIndexFunctionality(
//        const std::vector<std::string>& docs,
//        const std::vector<std::string>& requests,
//        const std::vector<std::vector<Entry>>& expected
//) {
//    std::vector<std::vector<Entry>> result;
//    InvertedIndex idx;
//    idx.updateDocumentBase(docs);
//    for(auto& request : requests) {
//        std::vector<Entry> word_count = idx.getWordCount(request);
//        result.push_back(word_count);
//    }
//    ASSERT_EQ(result, expected);
//}
//
//TEST(TestCaseInvertedIndex, TestBasic) {
//const std::vector<std::string> docs = {
//        "london is the capital of great britain",
//        "big ben is the nickname for the Great bell of the striking clock"
//};
//const std::vector<std::string> requests = {"london", "the"};
//const std::vector<std::vector<Entry>> expected = {
//        {
//                {0, 1}
//        }, {
//                {0, 1}, {1, 3}
//        }
//};
//TestInvertedIndexFunctionality(docs, requests, expected);
//}
//
//TEST(TestCaseInvertedIndex, TestBasic2) {
//    const std::vector<std::string> docs = {
//            "milk milk milk milk water water water",
//            "milk water water",
//            "milk milk milk milk milk water water water water water",
//            "americano cappuccino"
//    };
//    const std::vector<std::string> requests = {"milk", "water", "cappuccino"};
//    const std::vector<std::vector<Entry>> expected = {
//            {
//                    {0, 4}, {1, 1}, {2, 5}
//            }, {
//                    {0, 3}, {1, 2}, {2, 5}
//            }, {
//                    {3, 1}
//            }
//    };
//    TestInvertedIndexFunctionality(docs, requests, expected);
//}
//
//TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
//    const std::vector<std::string> docs = {
//            "a b c d e f g h i j k l",
//            "statement"
//    };
//    const std::vector<std::string> requests = {"m", "statement"};
//    const std::vector<std::vector<Entry>> expected = {
//            {
//            }, {
//                    {1, 1}
//            }
//    };
//    TestInvertedIndexFunctionality(docs, requests, expected);
//}
//
//TEST(TestCaseSearchServer, TestSimple) {
//    const std::vector<std::string> docs = {
//            "milk milk milk milk water water water",
//            "milk water water",
//            "milk milk milk milk milk water water water water water",
//            "americano cappuccino"
//    };
//    const std::vector<std::string> request = {"milk water", "sugar"};
//    const std::vector<std::vector<RelativeIndex>> expected = {
//            {
//                    {2, 1},
//                    {0, 0.7},
//                    {1, 0.3}
//            },
//            {
//            }
//    };
//    InvertedIndex idx;
//    idx.updateDocumentBase(docs);
//    SearchServer srv(idx);
//    std::vector<std::vector<RelativeIndex>> result = srv.search(request, 5);
//    ASSERT_EQ(result, expected);
//}
//
//TEST(TestCaseSearchServer, TestTop5) {
//    const std::vector<std::string> docs = {
//            "london is the capital of great britain",
//            "paris is the capital of france",
//            "berlin is the capital of germany",
//            "rome is the capital of italy",
//            "madrid is the capital of spain",
//            "lisboa is the capital of portugal",
//            "bern is the capital of switzerland",
//            "moscow is the capital of russia",
//            "kiev is the capital of ukraine",
//            "minsk is the capital of belarus",
//            "astana is the capital of kazakhstan",
//            "beijing is the capital of china",
//            "tokyo is the capital of japan",
//            "bangkok is the capital of thailand",
//            "welcome to third rome moscow is the capital of russia",
//            "amsterdam is the capital of netherlands",
//            "helsinki is the capital of finland",
//            "oslo is the capital of norway",
//            "stockholm is the capital of sweden",
//            "riga is the capital of latvia",
//            "tallinn is the capital of estonia",
//            "warsaw is the capital of poland",
//    };
//    const std::vector<std::string> request = {"moscow is the capital of russia"};
//    const std::vector<std::vector<RelativeIndex>> expected = {
//            {
//                    {7, 1},
//                    {14, 1},
//                    {0, 0.666666687},
//                    {1, 0.666666687},
//                    {2, 0.666666687}
//            }
//    };
//    InvertedIndex idx;
//    idx.updateDocumentBase(docs);
//    SearchServer srv(idx);
//    std::vector<std::vector<RelativeIndex>> result = srv.search(request, 5);
//    ASSERT_EQ(result, expected);
//}
//
//TEST(TestCaseSearchServer, Testletters) {
//    const std::vector<std::string> docs = {
//            "a b",
//            "b c f g",
//            "a b c d f g",
//            "d f a a",
//            "b",
//            "e",
//            "m"
//    };
//    const std::vector<std::string> request = {"a b c d e"};
//    const std::vector<std::vector<RelativeIndex>> expected = {
//            {
//                    {2, 1},
//                    {0, 0.5},
//                    {1, 0.5}
//            }
//    };
//    InvertedIndex idx;
//    idx.updateDocumentBase(docs);
//    SearchServer srv(idx);
//    std::vector<std::vector<RelativeIndex>> result = srv.search(request, 3);
//    ASSERT_EQ(result, expected);
//}
