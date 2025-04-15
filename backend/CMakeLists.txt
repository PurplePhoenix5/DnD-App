# Mindestens erforderliche CMake-Version. 3.10 ist ein guter Kompromiss zwischen
# Verfügbarkeit auf älteren Systemen und modernen Features. Im DevContainer ist
# eh eine aktuelle Version, aber es ist gute Praxis.
cmake_minimum_required(VERSION 3.10)

# Definiere den Projektnamen und die verwendete Hauptsprache (C++)
# Der Projektname wird z.B. für Lösungsvariablen in IDEs verwendet.
project(DnDAppBackend VERSION 1.0 LANGUAGES CXX) # CXX steht für C++

# C++ Standard festlegen (z.B. C++17)
# Wir wollen C++17 oder neuer.
set(CMAKE_CXX_STANDARD 17)
# Erzwinge den Standard (Build schlägt fehl, wenn der Compiler es nicht kann)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Verbiete Compiler-spezifische Erweiterungen (gut für Portabilität)
set(CMAKE_CXX_EXTENSIONS False)

# --- Externe Bibliotheken finden ---

# Finde die cmark Bibliothek
# Wir haben sie im Dockerfile mit 'apt-get install libcmark-dev' installiert.
# CMake sollte sie finden können. 'REQUIRED' bedeutet, dass CMake fehlschlägt,
# wenn die Bibliothek nicht gefunden wird.
find_package(cmark REQUIRED)

# --- Verzeichnisse für Header-Dateien hinzufügen ---

# Füge das 'include'-Verzeichnis hinzu (falls du dort Header ablegst)
# PUBLIC: Header sind auch für Dinge sichtbar, die dieses Target linken.
# PRIVATE: Header sind nur für dieses Target sichtbar.
# INTERFACE: Header sind nur für Dinge sichtbar, die dieses Target linken.
# Für ein Executable ist PRIVATE oft ausreichend.
# target_include_directories(DnDApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include) # Momentan noch nicht nötig

# Füge das 'external'-Verzeichnis hinzu, damit der Compiler nlohmann/json.hpp findet
target_include_directories(DnDApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)

# --- Dein Programm definieren ---

# Definiere die ausführbare Datei (Executable)
# Name des Executables: DnDApp
# Quelldateien: src/main.cpp (füge hier weitere .cpp-Dateien hinzu, wenn nötig)
add_executable(DnDApp
    src/main.cpp
    # Fügen Sie hier weitere .cpp Dateien hinzu, z.B.:
    # src/combat_tracker.cpp
    # src/encounter_builder.cpp
)

# --- Bibliotheken linken ---

# Linke die gefundenen Bibliotheken gegen dein Executable
# nlohmann/json ist header-only, muss nicht gelinkt werden.
# cmark muss gelinkt werden. find_package(cmark) erstellt ein "Imported Target"
# namens cmark::cmark, das wir hier verwenden.
target_link_libraries(DnDApp PRIVATE cmark::cmark)

# --- Optional: Ausgabeort festlegen ---
# Setze das Verzeichnis, in dem das kompilierte Executable landet.
# ${CMAKE_BINARY_DIR} ist das Haupt-Build-Verzeichnis (z.B. backend/build).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- Optional: Weitere Konfigurationen ---
# Hier könnten z.B. Compiler-Flags gesetzt werden für Debug/Release Builds etc.
# message(STATUS "Projekt ${PROJECT_NAME} wird konfiguriert.") # Beispiel für eine Meldung beim Konfigurieren