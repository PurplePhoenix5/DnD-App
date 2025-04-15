# Mindestens erforderliche CMake-Version
cmake_minimum_required(VERSION 3.11)

# Definiere den Projektnamen und die verwendete Hauptsprache (C++)
project(DnDAppBackend VERSION 1.0 LANGUAGES CXX)

# C++ Standard festlegen (z.B. C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# --- FetchContent Modul einbinden ---
include(FetchContent)

# --- Externe Bibliotheken deklarieren und holen ---

# Deklariere Crow
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG        v1.0 # Oder main, je nach Wahl
)

# Deklariere nlohmann/json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3 # Verwende einen bekannten, gültigen Tag (v3.11.3 ist aktuell stabil)
                       # v3.12.0 existiert nicht, nimm z.B. v3.11.3
)

# Stelle die deklarierten Inhalte zur Verfügung
FetchContent_MakeAvailable(crow)
FetchContent_MakeAvailable(nlohmann_json) # <-- DIESE ZEILE HINZUFÜGEN/ENTKOMMENTIEREN

# Finde System-Bibliotheken
find_package(cmark REQUIRED)
find_package(Threads REQUIRED)
# Asio wird von Crow intern gefunden, da wir libasio-dev installiert haben

# --- Dein Programm definieren ---
add_executable(DnDApp
    src/main.cpp
)

# --- Target Eigenschaften setzen (NACH add_executable) ---

# Füge Include-Verzeichnisse hinzu
target_include_directories(DnDApp PRIVATE
    ${crow_SOURCE_DIR}/include
    ${nlohmann_json_SOURCE_DIR}/include # Dieses Verzeichnis wird jetzt korrekt erstellt
)

# Linke die benötigten Bibliotheken
target_link_libraries(DnDApp PRIVATE
    cmark::cmark
    Threads::Threads
)

# --- Optional: Ausgabeort festlegen ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- Optional: Meldung beim Konfigurieren ---
message(STATUS "Projekt ${PROJECT_NAME} erfolgreich konfiguriert.")
message(STATUS "Crow source directory: ${crow_SOURCE_DIR}")
message(STATUS "nlohmann_json source directory: ${nlohmann_json_SOURCE_DIR}")