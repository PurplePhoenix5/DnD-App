# Mindestens erforderliche CMake-Version
cmake_minimum_required(VERSION 3.10)

# Definiere den Projektnamen und die verwendete Hauptsprache (C++)
project(DnDAppBackend VERSION 1.0 LANGUAGES CXX)

# C++ Standard festlegen (z.B. C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# --- Externe Bibliotheken finden ---
find_package(cmark REQUIRED)

# --- Dein Programm definieren ---
# HIER WIRD DAS TARGET "DnDApp" ERZEUGT
add_executable(DnDApp
    src/main.cpp
    # Fügen Sie hier weitere .cpp Dateien hinzu, z.B.:
    # src/combat_tracker.cpp
    # src/encounter_builder.cpp
)

# --- Target Eigenschaften setzen (NACH add_executable) ---

# Füge Verzeichnisse hinzu, in denen der Compiler nach Header-Dateien suchen soll.
# Das bezieht sich jetzt auf das gerade definierte Target 'DnDApp'.
target_include_directories(DnDApp PRIVATE
    # Das 'external'-Verzeichnis für nlohmann/json.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    # Das 'include'-Verzeichnis (falls du dort eigene Header ablegst)
    # ${CMAKE_CURRENT_SOURCE_DIR}/include # Kommentar entfernen, wenn du es nutzt
)

# Linke die gefundenen Bibliotheken gegen dein Executable
target_link_libraries(DnDApp PRIVATE cmark::cmark)

# --- Optional: Ausgabeort festlegen ---
# Setze das Verzeichnis, in dem das kompilierte Executable landet.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- Optional: Meldung beim Konfigurieren ---
message(STATUS "Projekt ${PROJECT_NAME} erfolgreich konfiguriert.")