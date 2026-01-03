# Goal
create a GUI app using Qt, and a free api

# Plan
1. 🚧 Terminal
2. ❌ SFML Window
3. ❌ Qt App (download .svg's instead of png's)

# Roadmap
0. 🚧 Reach MVP
  0.1. 🚧 Create Required Classes
    0.1.1 🚧 Create Guess Struct
      0.1.1.1. 🚧 Create Defined Methods
        0.1.1.1.2. 🚧 getCountryCodes
        0.1.1.1.3. 🚧 getFlagImage
        0.1.1.1.4. 🚧 displayOptions
        0.1.1.1.5. 🚧 getUserChoice
        0.1.1.1.6. 🚧 correctGuess
      0.1.1.2. 🚧 Standardize Variables
    0.1.2. ❌ Create Cache Struct
  0.2. 🚧 Create Required Functions
    0.2.1. ✅ Curl Callback
    0.2.2. ✅ Curl Connection Cleanup
  0.3. ❌ Create Main Function
  0.4. Come Up With A Way To Get RANDOM_COUNTRY_CODE
1. ❌ Add Cache Directory to Cache Struct
  1.1. ❌ Save codes.json
  2.2. ❌ Save Previously Fetched Images
  2.3. ❌ Keep Log Of Avaliable Images
2. ❌ Add Destructor to Guess Struct
  ❌ 2.1. Deletion Of Created Files
  ❌ 2.2. Deletion Of Created Cache