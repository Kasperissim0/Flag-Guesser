#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <SFML/Graphics.hpp>
#include "random.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

static const string website = "https://flagcdn.com", 
                          E = "❌ ERROR: ";

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
    Data() noexcept {
        curl_global_init(CURL_GLOBAL_DEFAULT); connection = curl_easy_init();
        curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, Data::dataProcessing);
        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &rawResponse);
        curl_easy_setopt(connection, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(connection, CURLOPT_FOLLOWLOCATION, 1L);
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
        const string request = website + "/en/codes.json";
        curl_easy_setopt(connection, CURLOPT_URL, request.c_str()); if (!performRequest()) throw runtime_error("Invalid Response Code Encountered");
        try { return json::parse(rawResponse); }
        catch (const json::parse_error& e) {
            cerr << E << e.what() << endl
                 << "👀 Raw Data: " << endl << rawResponse << endl;
            return nullopt;
        }
        catch (const json::other_error& e) {
            cerr << E << e.what() << endl
                 << "👀 Raw Data: " << endl << rawResponse << endl;
            return nullopt;
        }
        catch (const json::type_error& e) {
            cerr << E << e.what() << endl
                 << "👀 Raw Data: " << endl << rawResponse << endl;
            return nullopt;
        }
    }
    fs::path getFlagImage() {
        const string request = website + "/256x192/" + RANDOM_COUNTRY_CODE + ".png"; ofstream file("Cache/" + RANDOM_COUNTRY_CODE + ".svg", ios::binary);
        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(connection, CURLOPT_URL, request.c_str()); if (!performRequest()) throw runtime_error("Invalid Response Code Encountered");
        curl_easy_setopt(connection, CURLOPT_WRITEDATA, &rawResponse);
        return file;
    }
    ~Data() noexcept {
        curl_global_cleanup(); curl_easy_cleanup(connection);
        cout << "✅ Curl Connection Terminated" << endl;
    }
}
struct Round {
  fs::path image; string countryTitle, countryCode;
  static Data data;
  // bool clearCreated = false, 
  //      clearCache = false;
  
  Round() {
    try {
        image = data.getFlagImage();
        countryTitle = data.getCountryCodes()[RANDOM_COUNTRY_CODE];
        countryCode = RANDOM_COUNTRY_CODE;
    } catch (const runtime_error& e) {
        cerr << E << "Unable To Start A Round, Since: " << e.what() << endl 
             << "💥 Rethrowing Error" << endl;
        throw e;
    }
  }
  Round& displayRound () {
    return *this;
  }
  string getUserChoice() {

  }
  bool correctGuess(const string&& userChoice) {

  }
  // ~Round() {
  //   std::error_code e; uintmax_t filesDeleted;
  //   if (clearCache && filesDeleted = fs::remove_all("Cache", e)) 
  //   else if (clearCreated && image && !fs::remove(*image, e)) cerr << E << "Encountered Attempting To Delete " 
  //                                                                  << *image << endl 
  //                                                                  << e.message() << endl;
  // }
}
class Game {
    size_t roundsPlayed = 0, 
           correctGuesses = 0;
    // Round* currentRound;
    Game() : currentRound{new Round} {

    }
    Game& displayInterface () {
        return *this;
    }
    void playRound() { ++roundsPlayed;
        Round* round = new Round;

        // open flag image
        sf::RenderWindow w(sf::VideoMode(800, 600), "Flag #" + to_string((roundsPlayed + 1)));
        sf::Texture t; if (!t.loadFromFile(round->image)) throw runtime_error("Unable To Load Flag") 
        sf::Sprite s(t);
        while (w.isOpen()) {
            while (const auto e = w.pollEvent()) {
                if (e->is<sf::Event::Closed>()) w.close();
                if (const auto* k = e->getIf<sf::Event::KeyPressed>()) {
                    if (k->code == sf::Keyboard::Key::Escape) w.close()
                }
            }
            w.clear(); w.draw(s); w.display();
        }
        // display (5) options // get input // acess input
        if (round->correctGuess(round->displayRound().getUserChoice())) ++correctGuesses;

        delete round;
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