#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <optional>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <SFML/Graphics.hpp>

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

static const string E = "❌ ERROR: ";

struct Guess {
  fs::path* image = nullptr; string* country = nullptr;
  
  optional<json> getCountryCodes() {

  }
  optional<fs::path> getFlagImage() {
    const string website = "https://flagcdn.com",
                 fileType = ".svg";
  }
  string getChoice () {

  }
  bool correctGuess(const string& userChoice) {

  }
  ~Guess() {
    std::error_code e;
    if (image && !fs::remove(*image, e)) cerr << E << "Encountered Attempting To Delete " << *image << endl 
                                              << e.message() << endl;
  }
}

size_t cleanUp(CURL* c, bool normalExit = false) {
    curl_global_cleanup(); curl_easy_cleanup(c);
    cout << "✅ Curl Connection Terminated" << endl;
    return (normalExit ? 0 : 1);
}
size_t dataProcessing(void *more, size_t size, size_t amount, string* current) {
    size_t processed = size * amount;
    try {
        current->append(static_cast<char*>(more), processed);
        return processed;
    } catch (const bad_alloc& e) {
        cerr << E << e.what() << endl;
        return 0;
    }
}



int main(int argc, char** args) {
    CURL* connection; CURLcode responseCode; 
    string rawResponse; json parsedResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    connection = curl_easy_init(); if (!connection) return cleanUp(connection);
    curl_easy_setopt(connection, CURLOPT_URL, url.c_str());
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, dataProcessing);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &rawResponse);
    curl_easy_setopt(connection, CURLOPT_FOLLOWLOCATION, 1L);
    responseCode = curl_easy_perform(connection);
    if (responseCode != CURLE_OK) {
        cerr << E << curl_easy_strerror(responseCode) << endl;
        return cleanUp(connection);
    }
    try { parsedResponse = json::parse(rawResponse); }
    catch (const json::parse_error& e) {
        cerr << E << e.what() << endl
             << "👀 Raw Data Passed: " << endl << rawResponse << endl;
        return cleanUp(connection); 
    }

    try {
        cout << "Reason: " << parsedResponse["reason"] << endl;
    }  catch (const json::parse_error& e) {
        cerr << E << e.what() << endl;
        return cleanUp(connection); 
    } catch (const json::other_error& e) {
        cerr << E << e.what() << endl;
        return cleanUp(connection); 
    }
    
    return cleanUp(connection, true);
}