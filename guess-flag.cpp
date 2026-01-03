#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include "random.hpp"
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <SFML/Graphics.hpp>

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

const string website = "https://flagcdn.com", 
                   E = "❌ ERROR: ";
const fs::path cacheDirectory("~/Code/Projects/Flag\\ Guesser/Cache");

struct Data {
    CURL* connection; CURLcode responseCode; 
    string rawResponse; // json parsedResponse;

    static size_t dataProcessing(void *more, size_t size, size_t amount, string* current) noexcept {
        size_t processed = size * amount;
        try {
            current->append(static_cast<char*>(more), processed);
            return processed;
        } catch (const bad_alloc& e) {
            cerr << E << e.what() << endl;
            return 0;
        }
    }
    Data() {
        curl_global_init(CURL_GLOBAL_DEFAULT); connection = curl_easy_init();
        if (connection) clog << "✅ Curl Connection Established" << endl;
        else            throw runtime_error("Unable To Establish Curl Connection");
        curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, Data::dataProcessing);
        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &rawResponse);
        curl_easy_setopt(connection, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(connection, CURLOPT_FOLLOWLOCATION, 1L);
        clog << "✅ Curl Connection Configured" << endl;
    }
    bool performRequest() noexcept {
        responseCode = curl_easy_perform(connection);
        if (responseCode != CURLE_OK) {
            cerr << E << curl_easy_strerror(responseCode) << endl;
            return false;
        }
        return true;
    }
    json getCountryCodes() {
        const string title = "codes.json"; ofstream file(cacheDirectory + "/" + title, ios::out); json parsedResponse;
        if (fs::exists(file)) {
            ifstream content(file); string temp; while(getline(content, temp));
            try {
                parsedResponse = json::parse(temp); clog << "✅ Cached Request Read" << endl;
                return parsedResponse;
            }
            catch (const json::parse_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << temp << endl;
                throw runtime_error("Could Not Parse JSON");
            }
            catch (const json::other_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << temp << endl;
                throw runtime_error("Could Not Parse JSON");
            }
            catch (const json::type_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << temp << endl;
                throw runtime_error("Could Not Parse JSON");
            }
        }
        else {
            const string request = website + "/en/" + title;
            curl_easy_setopt(connection, CURLOPT_URL, request.c_str());  clog << "✅ Curl Connection Reconfigured" << endl;
            if (!performRequest()) throw runtime_error("Invalid Response Code Encountered");
            try {
                parsedResponse = json::parse(rawResponse); clog << "✅ Request Parsed" << endl;
                file << parsedResponse.dump(2); clog << "✅ Saved Country Codes To Cache" << endl;
                return parsedResponse;
            }
            catch (const json::parse_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << rawResponse << endl;
                throw runtime_error("Could Not Parse JSON");
            }
            catch (const json::other_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << rawResponse << endl;
                throw runtime_error("Could Not Parse JSON");
            }
            catch (const json::type_error& e) {
                cerr << E << e.what() << endl
                     << "👀 Raw Data: " << endl << rawResponse << endl;
                throw runtime_error("Could Not Parse JSON");
            }
        }
    }
    fs::path getFlagImage() {
        //! replace request with: 'website + RANDOM_COUNTRY_CODE + ".png"'
        const string extension = ".png", 
                     request = website + "/256x192/" + RANDOM_COUNTRY_CODE + extension;
        fs::path     file(cacheDirectory + "/" + RANDOM_COUNTRY_CODE + extension); 
        ofstream     content(file, ios::binary);

        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &content);
        clog << "✅ Curl Connection Reconfigured" << endl;
        curl_easy_setopt(connection, CURLOPT_URL, request.c_str()); if (!performRequest()) throw runtime_error("Invalid Response Code Encountered");
        clog << "✅ Content Retrieved And Saved In: " << file << endl;
        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &rawResponse);
        clog << "✅ Curl Connection Reconfigured" << endl;

        return file;
    }
    ~Data() noexcept {
        curl_global_cleanup(); curl_easy_cleanup(connection);
        clog << "✅ Curl Connection Terminated" << endl;
    }
}
struct Round {
  fs::path image; string countryTitle, countryCode;
  size_t amountOfChoices; static Data data; static const json codes = data.getCountryCodes();
  // bool clearCreated = false, 
  //      clearCache = false;
  
  Round(const size_t& choices) : amountOfChoices{choices} {
    try {
        image = data.getFlagImage();
        countryTitle = codes[RANDOM_COUNTRY_CODE];
        countryCode = RANDOM_COUNTRY_CODE;
    } catch (const runtime_error& e) {
        cerr << E << "Unable To Start A Round, Since: " << e.what() << endl 
             << "💥 Rethrowing Error" << endl;
        throw e;
    }
  }
  string displayRound () {
    size_t correctAnswer = gen.generate<size_t>(1, amountOfChoices); string temp;
    cout << "Possible Answers: " << endl;
    for (size_t i = 0; i < amountOfChoices; ++i) 
        cout << (i + 1) << ". " 
             << ((i == correctAnswer) ? (codes[countryCode] + " (" + countryCode + ")") : (codes[RANDOM_COUNTRY_CODE] + " (" + RANDOM_COUNTRY_CODE + ")")) 
             << endl;
    cout << endl << " ➡︎ Choice: "; cin >> temp;
    return temp;
  }
  bool correctGuess(const string&& userChoice) {
     // if the input is from 1 to amountOfChoices OR if it is the country code OR the country title process it
     // otherwise error
     {
        string number = ""; size_t n;
        for (const auto& c : userChoice)
            if (isdigit(c)) number += c;
        try {
            n = stoul(number);
            if (n > amountOfChoices) throw runtime_error("Number Entered Is Too Large");
        }
        catch () {}
        catch () {}
        catch () {}
    }
    // find if key applicable
    if (codes[userChoice])
    // ...
    // find if value applicable
    for (const auto& v : codes)
        if (v == userChoice)
    // ...

    

    
  }
  // ~Round() {
  //   std::error_code e; uintmax_t filesDeleted;
  //   if (clearCache && filesDeleted = fs::remove_all(cacheDirectory, e)) cerr << E << "Encountered Attempting To Recursively Delete Directory" 
  //                                                                            << cacheDirectory << endl 
  //                                                                            << e.message() << endl;
  //   else if (clearCreated && image && !fs::remove(image, e)) cerr << E << "Encountered Attempting To Delete " 
  //                                                                  << *image << endl 
  //                                                                  << e.message() << endl;
  // }
}
class Game {
    size_t roundsPlayed = 0, 
           correctGuesses = 0,
           amountOfOptions = 3;
    Round** currentRound;
    Game() {

    }
    Game& displayInterface () {

        return *this;
    }
    void playRound() {
        ++roundsPlayed; amountOfOptions = (((roundsPlayed / 5) >= 1) ? ((roundsPlayed / 5) + 2) : 3);
        Round* round = new Round(amountOfOptions); currentRound = &round;

        // open flag image
        sf::RenderWindow w(sf::VideoMode(800, 600), "Flag #" + to_string((roundsPlayed + 1)));
        sf::Texture t; if (!t.loadFromFile(round->image)) throw runtime_error("Unable To Load Flag Image") 
        sf::Sprite s(t);
        while (w.isOpen()) {
            while (const auto e = w.pollEvent()) {
                if (e->is<sf::Event::Closed>()) w.close();
                if (const auto* k = e->getIf<sf::Event::KeyPressed>()) {
                    if (k->code == sf::Keyboard::Key::Escape) w.close();
                }
            }
            w.clear(); w.draw(s); w.display();
        }
        // display (5) options // get input // acess input
        if (round->correctGuess(round->displayRound())) ++correctGuesses;

        delete round; currentRound = nullptr;
    }
    ~Game() {
        // free memory
    }

}


int main() { // int argc, char** args) {
    Game g;
    while (true) g.displayInterface().playRound();
    return 0;
}