#include <iostream>
#include <string>
#include <vector>

// Crow Header - oft reicht "crow.h", wenn über CMake eingebunden
#include "crow.h"

// nlohmann/json Header (stelle sicher, dass der Pfad stimmt,
// entweder über external/ oder FetchContent via target_include_directories)
#include "nlohmann/json.hpp"

// Für bequemeren Zugriff auf den json-Typ
using json = nlohmann::json;

int main() {
    // Erstelle das Crow App Objekt
    crow::SimpleApp app;

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
    CROW_ROUTE(app, "/api/monsters")([]() {
        json monster_list = json::array(); // Erstelle ein leeres JSON-Array

        // Füge Beispiel-Monster hinzu
        monster_list.push_back({
            {"id", "goblin_1"},
            {"name", "Goblin"},
            {"hp", 7},
            {"ac", 15}
        });
        monster_list.push_back({
            {"id", "skeleton_1"},
            {"name", "Skeleton"},
            {"hp", 13},
            {"ac", 13}
        });

        return crow::response(monster_list.dump());
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