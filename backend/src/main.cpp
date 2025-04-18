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

    CROW_ROUTE(app, "/api/encounters")([]() {
        json encounter_list = json::array(); // Erstelle ein leeres JSON-Array

        // Verzeichnis mit Encounter-Dateien
        const std::string encounters_dir = "../data/encounters";

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

    CROW_ROUTE(app, "/api/encounters/<string>")
        ([](const crow::request& /*req*/, const std::string& encounter_id) {
        //   ^--- `req` wird übergeben, aber nicht direkt genutzt (durch /*req*/ markiert)
        //        `encounter_id` enthält den Wert aus dem <string> Platzhalter

        // 1. Basispfad definieren (relativ zum Ausführungsverzeichnis, also 'build')
        const std::string base_dir = "../data/encounters";

        // 2. Vollständigen Dateipfad konstruieren
        //    Wir nehmen die encounter_id und hängen ".json" an.
        //    Verwende std::filesystem::path für sicherere Pfadoperationen.
        std::filesystem::path file_path;
        try {
             // Verwende / Operator von std::filesystem::path für plattformunabhängige Verknüpfung
            file_path = std::filesystem::path(base_dir) / (encounter_id + ".json");
             // Bereinige den Pfad (löst z.B. ../ auf, falls base_dir komplexer wäre)
            file_path = std::filesystem::absolute(file_path).lexically_normal();
        } catch (const std::exception& e) {
             std::cerr << "Fehler beim Erstellen des Dateipfads für ID '" << encounter_id << "': " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Erstellen des Dateipfads.\"}");
        }

        // Füge Logging hinzu, um den gesuchten Pfad zu sehen
        std::cout << "Suche nach Encounter-Datei: " << file_path << std::endl;


        // 3. Fehlerbehandlung mit try-catch für Dateisystem- und Leseoperationen
        try {
            // 4. Prüfen, ob die Datei existiert und eine reguläre Datei ist
            if (!std::filesystem::exists(file_path)) {
                 std::cerr << "Fehler: Datei nicht gefunden - " << file_path << std::endl;
                 json error_resp;
                 error_resp["error"] = "Encounter nicht gefunden.";
                 error_resp["requested_id"] = encounter_id;
                 error_resp["searched_path"] = file_path.string(); // Zum Debuggen hinzufügen
                 return crow::response(404, error_resp.dump());
            }
            if (!std::filesystem::is_regular_file(file_path)) {
                 std::cerr << "Fehler: Pfad ist keine reguläre Datei - " << file_path << std::endl;
                 json error_resp;
                 error_resp["error"] = "Angeforderter Pfad ist keine Datei.";
                 error_resp["requested_id"] = encounter_id;
                 error_resp["searched_path"] = file_path.string();
                 return crow::response(400, error_resp.dump()); // 400 Bad Request könnte hier passen
            }

            // 5. Datei öffnen
            std::ifstream file(file_path);
            if (!file.is_open()) {
                // Dieser Fall sollte selten sein, wenn exists() und is_regular_file() true waren,
                // könnte aber bei Berechtigungsproblemen auftreten.
                std::cerr << "Fehler: Datei konnte nicht geöffnet werden (Berechtigungen?) - " << file_path << std::endl;
                json error_resp;
                error_resp["error"] = "Encounter-Datei konnte nicht geöffnet werden.";
                return crow::response(500, error_resp.dump());
            }

            // 6. Gesamten Dateiinhalt in einen String lesen
            std::stringstream buffer;
            buffer << file.rdbuf(); // Effizientes Lesen des gesamten Inhalts
            std::string file_content = buffer.str();

            // 7. Erfolgreiche Antwort senden
            //    Wir senden den rohen JSON-String aus der Datei.
            //    Setze den Content-Type Header korrekt.
            crow::response res(200, file_content);
            res.set_header("Content-Type", "application/json");
            return res;

        } catch (const std::filesystem::filesystem_error& e) {
            // Fängt Fehler von std::filesystem::exists, is_regular_file etc. ab
            std::cerr << "Dateisystemfehler beim Zugriff auf " << file_path << ": " << e.what() << std::endl;
            json error_resp;
            error_resp["error"] = "Interner Dateisystemfehler.";
            return crow::response(500, error_resp.dump());
        } catch (const std::exception& e) {
            // Fängt andere mögliche Fehler ab (z.B. von std::stringstream)
            std::cerr << "Allgemeiner Fehler beim Verarbeiten der Datei " << file_path << ": " << e.what() << std::endl;
            json error_resp;
            error_resp["error"] = "Interner Serverfehler beim Verarbeiten der Anfrage.";
            return crow::response(500, error_resp.dump());
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