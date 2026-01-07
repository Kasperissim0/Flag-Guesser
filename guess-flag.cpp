//§ Includes
    #include <cctype>
    #include <cstddef>
    #include <cstdlib>
    #include <iterator>
    #include <optional>
    #include <string>
    #include <fstream>
    #include <iostream>
    #include <stdexcept>
    #include <filesystem>
    #include <sstream>
    #include "random.hpp"
    #include "wait.hpp"
    #include "screen.hpp"
    #include <curl/curl.h>
    #include "nlohmann/json.hpp"
    #include <pwd.h>
    #include <unistd.h>

//.
//§ Shortucts
    using namespace std;
    using json = nlohmann::json;
    namespace fs = filesystem;
//.
//§ Constants
    const string website = "https://flagcdn.com", 
                    E = "❌ ERROR: ",
                    C = "  ➡︎ Choice: ";
    const fs::path cacheDirectory([]{
        string fullPath = [] -> fs::path {
            if (const char* home = getenv("HOME")) return home;
            if (struct passwd* pw = getpwuid(getuid())) return pw->pw_dir;

            throw runtime_error("Unable to determine home directory");
        }(); 
        fullPath += "/Code/Projects/Flag Guesser/Cache";
        return fs::path(fullPath);
    }());
//.
//§ Classes
    struct UserAnswer {
        string code, name;
    };
    struct Data {
        //§ Variables
        CURL* connection; CURLcode responseCode;
        string rawResponse = "";
        //.

        static size_t API_StringProcessing(void *more, size_t size, size_t amount, string* current) noexcept {
            size_t processed = size * amount;
            try {
                current->append(static_cast<char*>(more), processed);
                return processed;
            } 
            catch (const bad_alloc& e) {
                cerr << E << e.what() << endl;
                return 0;
            }
        }
        static auto getRandomIterator(const json& JSON_Object) {
            if (!JSON_Object.is_object())     throw runtime_error("Country Codes Are Not An Object");
            if (JSON_Object.empty())          throw runtime_error("Country Codes Are Empty");

            size_t randomIteratorMoveAmount = gen.generate<size_t>(0, (JSON_Object.size() - 1));
            auto iterator = JSON_Object.cbegin(); advance(iterator, randomIteratorMoveAmount);
            return iterator;
        }

        Data() {
            curl_global_init(CURL_GLOBAL_DEFAULT); connection = curl_easy_init();
            if (!connection) throw runtime_error("Unable To Establish Curl Connection");
            curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, Data::API_StringProcessing);
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
            const string title = "codes.json"; json parsedResponse; 
            fs::path pathToFile = cacheDirectory.string() + "/" + title; 
            
            if (!fs::exists(cacheDirectory)) fs::create_directories(cacheDirectory);
            if (fs::exists(pathToFile)) {
                ifstream fileContent(pathToFile); string rawContent = "";

                if (!fileContent.is_open()) throw runtime_error("Could Not Open " + pathToFile.string());
                try {
                    fileContent >> parsedResponse;
                    fileContent.close(); return parsedResponse;
                }
                catch (const json::parse_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawContent << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
                catch (const json::other_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawContent << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
                catch (const json::type_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawContent << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
            }
            else {
                const string request = website + "/en/" + title; ofstream fileContent(pathToFile, ios::out);

                curl_easy_setopt(connection, CURLOPT_URL, request.c_str());
                if (!performRequest())       throw runtime_error("Invalid Response Code Encountered");
                if (!fileContent.is_open())  throw runtime_error("Could Not Open " + pathToFile.string()); 
                try {
                    parsedResponse = json::parse(rawResponse); fileContent << parsedResponse.dump(2);
                    fileContent.close(); return parsedResponse;
                }
                catch (const json::parse_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawResponse << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
                catch (const json::other_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawResponse << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
                catch (const json::type_error& e) {
                    cerr << E << e.what() << endl
                         << "👀 Raw Data: " << endl << rawResponse << endl;
                    fileContent.close(); throw runtime_error("Could Not Parse JSON");
                }
            }
        }
        fs::path getFlagImage(optional<string> countryCode = nullopt) {
            if (!countryCode) countryCode = Data::getRandomIterator(getCountryCodes()).key();
            const string extension = ".svg", request = website + "/" + *countryCode + extension; 
            fs::path file(cacheDirectory.string() + "/" + *countryCode + extension);

            if (!fs::exists(file)) {
                ofstream content(file, ios::trunc); rawResponse = "";

                curl_easy_setopt(connection, CURLOPT_URL, request.c_str()); 
                if (!performRequest())  throw runtime_error("Invalid Response Code Encountered");
                if (!content.is_open()) throw runtime_error("Could Not Open " + file.string());
                content << rawResponse; content.close();

            }
            return file;
        }
        ~Data() noexcept {
            curl_global_cleanup(); curl_easy_cleanup(connection);
        }
    };
    struct Round {
        //§ Variables
            struct Helpers {
                static Data data;
                // // inline static const json countryCodes = data.getCountryCodes();
                const json& countryCodes() {
                    static json codes = data.getCountryCodes();
                    return codes;
                }
            } helpers;
                fs::path image;       string countryTitle = "", 
                                             countryCode = "";
                size_t amountOfChoices; bool clearCreated = true, 
                                             clearCache = false; // TODO Add Ways Of Setting This
                vector<UserAnswer> answerOptions = {};
        //.
        
        Round(const size_t& choices) : amountOfChoices{choices} {
            try { for (size_t i = 0; i < amountOfChoices; ++i) answerOptions.push_back({"", ""});
                const auto randomCode = Data::getRandomIterator(helpers.countryCodes());
                image = helpers.data.getFlagImage(randomCode.key());
                countryTitle = randomCode.value().get<string>();
                countryCode = randomCode.key();
            } 
            catch (const runtime_error& e) {
                cerr << E << "Unable To Start A Round, Since: " << e.what() << endl 
                     << "💥 Rethrowing Error" << endl;
                throw e;
            }
        }
        string displayOptionsAndGetValidAnswer () noexcept {
            size_t correctAnswer = gen.generate<size_t>(1, amountOfChoices);
            stringstream choiceOptions; string userChoice;

            choiceOptions << endl << "Possible Answers: " << endl;
            for (size_t i = 1; i <= amountOfChoices; ++i) {
                const auto randomCountry = Data::getRandomIterator(helpers.countryCodes());
                const auto currentCode = ((i == correctAnswer) ? countryCode : randomCountry.key()),
                           currentTitle = ((i == correctAnswer) ? countryTitle : randomCountry.value().get<string>());

                // clog << "⚠️ Adding " << currentCode << " : " << currentTitle << endl;
                answerOptions.at(i - 1) = {currentCode, currentTitle};
                choiceOptions << " " << to_string((i)) << ". " 
                              << currentTitle << " (" << currentCode << ")" << endl;
            } choiceOptions   << endl << C;
            // clog << "⚠️ Amount Of Iterators Saved: " << answerOptions.size() << endl
            //      << "Specifically: " << endl;
            // for (const auto& u : answerOptions) clog << "- " << u.code << " : " << u.name << endl;
            
            do {
                userChoice = "";
                cout << choiceOptions.str(); cin >> userChoice;
            } while (![&] -> bool {
                bool validInput = false;

                // 1. Input Is A Country Code OR 2. Input Is A Country Title
                //! for (const auto& u : answerOptions) if (userChoice == u.code || userChoice == u.name) validInput = true;
                // clog << "⚠️ User Input: " << userChoice << endl << endl;
                for (const auto& u : answerOptions) {
                    // clog << "⚠️ Current JSON Value" << endl
                    //     << "- " << u.code << " : " << u.name << endl;
                    if (userChoice == u.code) {
                        // clog << "✅ Correct Input Since " << userChoice << " is equal to " << u.code << endl;
                        validInput = true; break;
                    }
                    if (userChoice == u.name) {
                        // clog << "✅ Correct Input Since " << userChoice << " is equal to " << u.name << endl;
                        validInput = true; break;
                    }
                }
                // 3. Input Is a Number (Index From 1 to amountOfChoices)
                string tempStorage = ""; size_t constructedNumber;
                for (const auto& c : userChoice) if (isdigit(c)) tempStorage.push_back(c);
                try {
                    constructedNumber = stoul(tempStorage);
                    if (constructedNumber <= amountOfChoices) validInput = true;
                } catch (...) {}

                if (!validInput) {
                    cout << endl << "❌ Invalid Input Entered" << endl;
                    // wait(3); clearScreen();
                }
                return validInput;
            }());
            // clog << "✅ Returning Value: " << userChoice << endl;
            return userChoice;
        }
        bool correctGuess(const string&& choice) noexcept {
            // clog << "⚠️ Processing User Guess: " << choice << endl << endl
            //      << "⚠️ Correct Answers Include: " << endl
            //      << "- " << countryCode << endl
            //      << "- " << countryTitle << endl
            //      << "- " << "1-" << (amountOfChoices - 1) << endl;
            if (countryCode == choice || countryTitle == choice) {
                // clog << "✅ The Users Guess Was Correct" << endl;
                return true;
            }

            size_t index;
            try {
                index = stoul(choice);
                if (answerOptions.at((index - 1)).code == countryCode || answerOptions.at((index - 1)).name == countryTitle) {
                    // clog << "✅ The Users Guess Was Correct" << endl;
                    return true;
                }
            }
            catch (...) { } // cerr << E << "Invalid Index Passed" << endl; }

            // clog << "❌ The Users Guess Was NOT Correct" << endl;
            return false;
        }
        ~Round() {
           error_code e; uintmax_t filesDeleted; // char temp;
            
           if (clearCreated) {
           //  cout << "Are You Sure You Want To Delete " << image << " (y/n)" << endl << C; cin >> temp;
           //  if (temp == 'y') {
                if (!fs::remove(image, e) && e) cerr << E << "Failed To Delete, Since: " << e.message() << endl;
                // else clog  << "✅ Successfully Deleted " << image << endl;
           //  }
            } 
           else if (clearCache) {
            //  cout << "Are You Sure You Want To Delete The Entire Contents Of Your Cache? (y/n)" << endl << C; cin >> temp;
            //  if (temp == 'y') {
                 filesDeleted = fs::remove_all(cacheDirectory, e);
                 // clog  << "✅ Removed " << filesDeleted << " Items From " << cacheDirectory << endl;
            //  }
            }
            cout << "✅ Round Over" << endl; wait(1); clearScreen();
         }
    }; Data Round::Helpers::data;
    struct Game {
        Round** currentRound;
        size_t roundsPlayed = 0, 
               correctGuesses = 0,
               amountOfOptions = 3;
        Game() {
            
        }
        Game& displayInterface () { // clearScreen();
            const string line = "------";
            cout << line << endl << line << endl
                 << "Games Played: " << roundsPlayed << endl 
                 << "Games Won: " << correctGuesses << endl
                 << "Win Percentage: " 
                 << (correctGuesses == 0 ? 0 : ((static_cast<double>(correctGuesses) / static_cast<double>(roundsPlayed)) * 100 )) << "%" << endl
                 << line << endl << line << endl;
            return *this;
        }
        void playRound() { ++roundsPlayed; 
            amountOfOptions = (((roundsPlayed / 5) >= 1) ? ((roundsPlayed / 3) + 2) : 3);
            Round* round = new Round(amountOfOptions); currentRound = &round;

            // open flag image
            string command = "code ", //? use code/open as the command
                   path = round->image;
            command += [&] -> string {
                for (size_t i = 0; i < path.size(); ++i)
                    if (isspace(path.at(i))) 
                        path.insert((path.begin() + i++), '\\');
                return path;
            }();
            system(command.c_str());
            /*
            sf::RenderWindow w(sf::VideoMode({800, 600}), "Flag #" + to_string((roundsPlayed + 1)));
            sf::Texture t; if (!t.loadFromFile(round->image)) throw runtime_error("Unable To Load Flag Image");
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
            */
            // display (5) options // get input // acess input
            if (round->correctGuess(round->displayOptionsAndGetValidAnswer())) { ++correctGuesses;
                 cout << "✅ Right Answer" << endl;
            }
            else cout << "❌ Wrong Answer" << endl;

            wait(1); delete round; currentRound = nullptr;
        }
        ~Game() {
            // free memory
        }

    };
//.
//§ Main Loop
    int main() {
        Game g;
        while (true) g.displayInterface().playRound();
        return 0;
    }
//.