#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // Für std::filesystem
#include <fstream>    // Für std::ifstream

// Crow Header - oft reicht "crow.h", wenn über CMake eingebunden
#include "crow.h"

// nlohmann/json Header 
#include "nlohmann/json.hpp"

struct CorsMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& /*ctx*/) {
        // Erlaube Anfragen von deinem Frontend-Entwicklungsserver
        res.add_header("Access-Control-Allow-Origin", "http://localhost:5173"); 
        
        // Erlaube bestimmte Methoden (GET, POST, OPTIONS sind häufig)
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        
        // Erlaube bestimmte Header in der Anfrage (z.B. für POST mit JSON)
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

        // Handle Preflight-Anfragen (Browser sendet OPTIONS vor komplexeren Anfragen wie POST)
        if (req.method == "OPTIONS"_method) {
            res.code = 204; // No Content
            res.end();      // Beende die Antwort hier für OPTIONS
            return;         // WICHTIG: Sofort zurückkehren!
        }
    }

    void after_handle(crow::request& /*req*/, crow::response& res, context& /*ctx*/) {
        // Stelle sicher, dass die CORS-Header in der endgültigen Antwort enthalten sind
        res.add_header("Access-Control-Allow-Origin", "http://localhost:5173");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};


// Für bequemeren Zugriff auf den json-Typ
using json = nlohmann::json;

int main() {
    // Erstelle das Crow App Objekt MIT Middleware
    crow::App<CorsMiddleware> app;

    // ----- API Endpunkte (Routen) definieren -----

    // 1. Einfacher Status-Endpunkt (GET /api/status)
    //    Zeigt, dass der Server läuft und antwortet.
    //    CROW_ROUTE definiert eine Route für eine bestimmte URL.
    //    Der Lambda-Ausdruck [](){ ... } wird ausgeführt, wenn die Route aufgerufen wird.
    CROW_ROUTE(app, "/api/status")([]() {
        // Erstelle ein JSON-Objekt für die Antwort
        json response;
        response["status"] = "OK";
        response["message"] = "DnD Backend ist bereit!";

        // Gib eine Crow-Antwort zurück.
        // .dump() wandelt das JSON-Objekt in einen String um.
        // Der Content-Type wird automatisch auf application/json gesetzt.
        return crow::response(response.dump());
    });

    // 2. Beispiel-Endpunkt für Monster (GET /api/monsters)
    //    Gibt eine feste Liste von Monstern zurück (später liest du das aus JSON-Dateien).
    CROW_ROUTE(app, "/api/encounters")([]() {
        json encounter_list = json::array(); // Erstelle ein leeres JSON-Array

        // Verzeichnis mit Encounter-Dateien
        const std::string encounters_dir = "backend/data/encounters";

        // Iteriere über alle Dateien im Verzeichnis
        for (const auto& entry : std::filesystem::directory_iterator(encounters_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                // Öffne die JSON-Datei
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    try {
                        // Lese den Inhalt der Datei in ein JSON-Objekt
                        json encounter_data;
                        file >> encounter_data;

                        // Extrahiere die Felder "id" und "name", falls vorhanden
                        if (encounter_data.contains("id") && encounter_data.contains("name")) {
                            encounter_list.push_back({
                                {"id", encounter_data["id"]},
                                {"name", encounter_data["name"]}
                            });
                        }
                    } catch (const std::exception& e) {
                        // Fehler beim Parsen der Datei - überspringe sie
                        std::cerr << "Fehler beim Lesen der Datei " << entry.path() << ": " << e.what() << std::endl;
                    }
                }
            }
        }

        // Gib die Liste als JSON-Antwort zurück
        return crow::response(encounter_list.dump());
    });

    // 3. Beispiel-Endpunkt, der einen Parameter aus der URL liest (GET /api/monsters/<monster_id>)
    //    <string> fängt den Teil der URL nach /api/monsters/ ab und übergibt ihn als String.
    CROW_ROUTE(app, "/api/monsters/<string>")([](const std::string& monster_id) {
        // In einer echten Anwendung: Suche das Monster mit dieser ID in deinen Daten.
        // Hier nur ein einfaches Beispiel:
        if (monster_id == "goblin_1") {
            json monster_data = {
                {"id", "goblin_1"},
                {"name", "Goblin"},
                {"hp", 7},
                {"ac", 15},
                {"description", "Ein kleiner, fieser Goblin."}
            };
            return crow::response(monster_data.dump());
        } else {
            // Monster nicht gefunden - gib einen 404 Fehler zurück
            json error_response;
            error_response["error"] = "Monster nicht gefunden";
            error_response["requested_id"] = monster_id;
            // Erstelle eine Antwort mit Statuscode 404 (Not Found)
            return crow::response(404, error_response.dump());
        }
    });

    // ----- Server Konfiguration und Start -----

    // Setze den Port, auf dem der Server lauschen soll (z.B. 8080)
    app.port(8080)
        // Aktiviere Multithreading, damit der Server mehrere Anfragen gleichzeitig bearbeiten kann
        .multithreaded()
        // Starte den Server. Dieser Aufruf blockiert, d.h. das Programm läuft hier weiter,
        // bis der Server gestoppt wird (z.B. mit Ctrl+C im Terminal).
        .run();

    // Dieser Teil wird erst erreicht, wenn der Server gestoppt wird.
    std::cout << "Server wird beendet." << std::endl;

    return 0;
}