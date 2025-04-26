#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // Für std::filesystem
#include <fstream>    // Für std::ifstream/ofstream
#include <sstream>    // Für std::stringstream
#include <exception>  // Für std::exception
#include <cmath>      // Für std::floor
#include <iomanip>    // Für std::setw (im dump für pretty print)

// Crow Header
#include "crow.h"
// nlohmann/json Header
#include "nlohmann/json.hpp"

// Deine CorsMiddleware (wie gehabt)
struct CorsMiddleware {
    // ... (Middleware Code wie zuvor) ...
     struct context {};
    void before_handle(crow::request& req, crow::response& res, context& /*ctx*/) {
        if (req.method == "OPTIONS"_method) {
            res.code = 204;
            res.end();
            return;
        }
    }
    void after_handle(crow::request& /*req*/, crow::response& res, context& /*ctx*/) {
        res.add_header("Access-Control-Allow-Origin", "http://localhost:5173");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};

using json = nlohmann::json;

// --- Hilfsfunktion zum Laden eines Monster-Statblocks ---
// Gibt ein leeres JSON-Objekt zurück, wenn Laden fehlschlägt oder Felder fehlen
json load_monster_statblock(const std::string& monster_id) {
    const std::string monsters_base_dir = "../data/monsters";
    std::filesystem::path monster_file_path;

    try {
        monster_file_path = std::filesystem::path(monsters_base_dir) / (monster_id + ".json");
        monster_file_path = std::filesystem::absolute(monster_file_path).lexically_normal();

        if (!std::filesystem::exists(monster_file_path) || !std::filesystem::is_regular_file(monster_file_path)) {
            std::cerr << "Fehler: Monster-Datei nicht gefunden oder keine Datei: " << monster_file_path << std::endl;
            return nullptr; // Signalisiert Fehler (oder leeres Objekt)
        }

        std::ifstream monster_file(monster_file_path);
        if (!monster_file.is_open()) {
            std::cerr << "Fehler: Monster-Datei konnte nicht geöffnet werden: " << monster_file_path << std::endl;
             return nullptr;
        }

        json monster_data;
        monster_file >> monster_data; // Parse JSON
        return monster_data;

    } catch (const json::parse_error& e) {
        std::cerr << "Fehler beim Parsen der Monster-JSON-Datei " << monster_file_path << ": " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Laden der Monster-Datei " << monster_file_path << ": " << e.what() << std::endl;
        return nullptr;
    }
}


int main() {
    crow::App<CorsMiddleware> app;
    const std::string encounters_base_dir = "../data/encounters";
    const std::string monsters_base_dir = "../data/monsters";
    const std::string dnddata_base_dir = "../data/DnDData";
    const std::string templates_base_dir = "../data/templates";

    // --- Simpler In-Memory Cache für DnDData ---
    // Verhindert das ständige Lesen kleiner Dateien vom Laufwerk
    std::map<std::string, json> dndDataCache;
    // --- Ende Cache ---

    // --- GET /api/status ---
    CROW_ROUTE(app, "/api/status")([]() { /* ... wie zuvor ... */
        json response;
        response["status"] = "OK";
        response["message"] = "DnD Backend ist bereit!";
        crow::response res(response.dump());
        res.set_header("Content-Type", "application/json");
        return res;
     });

    // --- GET /api/encounters ---
    CROW_ROUTE(app, "/api/encounters")([&]() { // Capture base_dir
        json encounter_list = json::array();
        try {
            for (const auto& entry : std::filesystem::directory_iterator(encounters_base_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        try {
                            json data;
                            file >> data;
                            if (data.contains("id") && data.contains("name")) {
                                encounter_list.push_back({{"id", data["id"]}, {"name", data["name"]}});
                            }
                        } catch (const std::exception& e) {
                             std::cerr << "Fehler beim Verarbeiten von " << entry.path() << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Auflisten von Encountern in " << encounters_base_dir << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Serverfehler beim Auflisten der Encounter.\"}");
        }
        crow::response res(encounter_list.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // --- GET /api/encounters/{id} ---
    // Gibt jetzt die *angereicherte* Version zurück, damit Combat Tracker sie hat
     CROW_ROUTE(app, "/api/encounters/<string>")
        ([&](const crow::request& /*req*/, const std::string& encounter_id) {
        std::filesystem::path encounter_file_path;
        try {
            encounter_file_path = std::filesystem::path(encounters_base_dir) / (encounter_id + ".json");
            encounter_file_path = std::filesystem::absolute(encounter_file_path).lexically_normal();

            if (!std::filesystem::exists(encounter_file_path) || !std::filesystem::is_regular_file(encounter_file_path)) {
                return crow::response(404, "{\"error\": \"Encounter nicht gefunden.\"}");
            }

            std::ifstream file(encounter_file_path);
            if (!file.is_open()) {
                 return crow::response(500, "{\"error\": \"Encounter-Datei konnte nicht geöffnet werden.\"}");
            }

            // Lese die *gespeicherte* Encounter-Datei (die bereits die angereicherten Infos enthält)
            json encounter_data;
            file >> encounter_data;

            crow::response res(encounter_data.dump());
            res.set_header("Content-Type", "application/json");
            return res;

        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen der Encounter-Datei " << encounter_file_path << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Fehler beim Lesen der Encounter-Daten.\"}");
        } catch (const std::exception& e) {
            std::cerr << "Allgemeiner Fehler beim Holen des Encounters " << encounter_id << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Interner Serverfehler.\"}");
        }
    });

     // --- GET /api/monsters/summary ---
     CROW_ROUTE(app, "/api/monsters/summary")([&]() { // Capture base_dir
        json monster_summary_list = json::array();
        try {
             for (const auto& entry : std::filesystem::directory_iterator(monsters_base_dir)) {
                 if (entry.is_regular_file() && entry.path().extension() == ".json") {
                     std::ifstream file(entry.path());
                     if (file.is_open()) {
                         try {
                             json data;
                             file >> data;
                             // Extrahiere nur die benötigten Felder für die Zusammenfassung
                             json summary_item;
                             summary_item["id"] = entry.path().stem().string(); // ID aus Dateinamen
                             summary_item["name"] = data.value("name", "Unknown");
                             summary_item["cr"] = data.value("CR", 0); // Verwende value für Default
                             summary_item["size"] = data.value("size", "Medium");
                             summary_item["type"] = data.value("type", "unknown");
                             monster_summary_list.push_back(summary_item);
                         } catch (const std::exception& e) {
                             std::cerr << "Fehler beim Verarbeiten der Monster-Datei " << entry.path() << ": " << e.what() << std::endl;
                             // Überspringe fehlerhafte Dateien
                         }
                     }
                 }
             }
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Auflisten der Monster in " << monsters_base_dir << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Serverfehler beim Auflisten der Monster.\"}");
        }
        crow::response res(monster_summary_list.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // --- GET /api/spells ---
    CROW_ROUTE(app, "/api/spells")([&]() { // Capture base_dirs if needed, though not directly used here
        const std::string spells_file_path_str = "../data/spells/spells.json"; // Pfad zur Datei
        std::filesystem::path spells_file_path;

        try {
            spells_file_path = std::filesystem::absolute(spells_file_path_str).lexically_normal();

            if (!std::filesystem::exists(spells_file_path) || !std::filesystem::is_regular_file(spells_file_path)) {
                std::cerr << "Fehler: Spells-Datei nicht gefunden: " << spells_file_path << std::endl;
                return crow::response(404, "{\"error\": \"Spell data file not found.\"}");
            }

            std::ifstream spells_file(spells_file_path);
            if (!spells_file.is_open()) {
                    std::cerr << "Fehler: Spells-Datei konnte nicht geöffnet werden: " << spells_file_path << std::endl;
                    return crow::response(500, "{\"error\": \"Could not open spell data file.\"}");
            }

            // Lese den gesamten Dateiinhalt in einen String
            std::stringstream buffer;
            buffer << spells_file.rdbuf();
            std::string spells_content = buffer.str();

            // Sende den rohen JSON-String zurück
            crow::response res(200, spells_content);
            res.set_header("Content-Type", "application/json");
            return res;

        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Laden der Spells-Datei (" << spells_file_path << "): " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error loading spell data.\"}");
        }
    });

    // --- GET /api/dnddata/{filename} ---
    CROW_ROUTE(app, "/api/dnddata/<string>")
        ([&](const std::string& requested_filename) { // Capture base_dir und Cache

        // === NEUE, SICHERERE VALIDIERUNG ===
        // 1. Prüfe auf leeren Dateinamen
        if (requested_filename.empty()) {
            return crow::response(400, "{\"error\": \"Filename cannot be empty.\"}");
        }

        // 2. Prüfe auf ungültige Zeichen (Path Traversal etc.)
        if (requested_filename.find("..") != std::string::npos || // Verhindere ".."
            requested_filename.find('/') != std::string::npos ||  // Verhindere Slashes
            requested_filename.find('\\') != std::string::npos) { // Verhindere Backslashes
             std::cerr << "Sicherheitswarnung: Ungültige Zeichen im Dateinamen angefordert: " << requested_filename << std::endl;
             return crow::response(400, "{\"error\": \"Invalid characters in filename.\"}");
        }

        // 3. Prüfe die Dateiendung (erlaube nur .json)
        if (requested_filename.length() <= 5 || requested_filename.substr(requested_filename.length() - 5) != ".json") {
             std::cerr << "Sicherheitswarnung: Ungültige Dateiendung angefordert: " << requested_filename << std::endl;
             return crow::response(400, "{\"error\": \"Invalid file extension. Only .json allowed.\"}");
        }
        // === ENDE VALIDIERUNG ===

        // Verwende den validierten Dateinamen direkt
        const std::string filename = requested_filename;


        // --- Optional: Cache Check ---
        if (dndDataCache.count(filename)) {
            crow::response res(dndDataCache[filename].dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "Cache"); // Zum Debuggen
            return res;
        }
        // --- Ende Cache Check ---

        std::filesystem::path file_path;
        std::filesystem::path absolute_base_path;
        try {
            file_path = std::filesystem::path(dnddata_base_dir) / filename;
            file_path = std::filesystem::absolute(file_path).lexically_normal();

            // Zusätzliche Sicherheitsprüfung: Stelle sicher, dass der Pfad immer noch im dnddata_base_dir liegt
            if (file_path.string().find(std::filesystem::absolute(dnddata_base_dir).string()) != 0) {
                 std::cerr << "Sicherheitswarnung: Versuchter Zugriff außerhalb von DnDData Verzeichnis: " << file_path << std::endl;
                 return crow::response(400, "{\"error\": \"Invalid file path requested.\"}");
            }


            if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
                std::cerr << "Fehler: DnDData-Datei nicht gefunden: " << file_path << std::endl;
                return crow::response(404, "{\"error\": \"Requested DnD data file not found.\"}");
            }

            std::ifstream data_file(file_path);
            if (!data_file.is_open()) {
                 std::cerr << "Fehler: DnDData-Datei konnte nicht geöffnet werden: " << file_path << std::endl;
                 return crow::response(500, "{\"error\": \"Could not open DnD data file.\"}");
            }

            json data_content;
            data_file >> data_content; // Direkt als JSON parsen

            // --- Optional: Zum Cache hinzufügen ---
            dndDataCache[filename] = data_content;
            // --- Ende Cache Add ---

            crow::response res(data_content.dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "File"); // Zum Debuggen
            return res;

        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen der DnDData-Datei " << file_path << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Error reading DnD data content.\"}");
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Laden der DnDData-Datei " << filename << " (" << file_path << "): " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error loading DnD data.\"}");
        }
    });

    // --- Routen für Templates (Platzhalter) ---
    CROW_ROUTE(app, "/api/templates/<string>")([](const std::string& type){ return crow::response(501, "Not Implemented"); }); // List
    CROW_ROUTE(app, "/api/templates/<string>/<string>")([](const std::string& type, const std::string& id){ return crow::response(501, "Not Implemented"); }); // Get One
    CROW_ROUTE(app, "/api/templates/<string>").methods("POST"_method)([](const crow::request& req, const std::string& type){ return crow::response(501, "Not Implemented"); }); // Create
    CROW_ROUTE(app, "/api/templates/<string>/<string>").methods("PUT"_method)([](const crow::request& req, const std::string& type, const std::string& id){ return crow::response(501, "Not Implemented"); }); // Update
    CROW_ROUTE(app, "/api/templates/<string>/<string>").methods("DELETE"_method)([](const std::string& type, const std::string& id){ return crow::response(501, "Not Implemented"); }); // Delete


    // --- POST /api/encounters (Erstellen/Aktualisieren) ---
    CROW_ROUTE(app, "/api/encounters").methods("POST"_method)
        ([&](const crow::request& req){ // Capture base_dirs
        json incoming_data;
        try {
            incoming_data = json::parse(req.body);
        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen des Request Body: " << e.what() << std::endl;
            return crow::response(400, "{\"error\": \"Ungültiges JSON im Request Body.\"}");
        }

        // 1. Daten validieren/extrahieren (Beispielhaft)
        if (!incoming_data.contains("name") || !incoming_data["name"].is_string() ||
            !incoming_data.contains("monsters") || !incoming_data["monsters"].is_array()) {
            return crow::response(400, "{\"error\": \"Fehlende oder ungültige Felder 'name' oder 'monsters'.\"}");
        }

        std::string encounter_id = incoming_data.value("id", ""); // ID aus Request oder leer
        std::string encounter_name = incoming_data["name"];
        bool is_new_encounter = encounter_id.empty();

        // Wenn keine ID vorhanden, generiere eine aus dem Namen (oder Timestamp)
        if (is_new_encounter) {
            encounter_id = encounter_name;
            // Einfache ID-Generierung: Kleinbuchstaben, ersetze Leerzeichen/Sonderzeichen durch '_'
            std::transform(encounter_id.begin(), encounter_id.end(), encounter_id.begin(), ::tolower);
            std::replace_if(encounter_id.begin(), encounter_id.end(), [](char c){ return !std::isalnum(c); }, '_');
            // Optional: Prüfen, ob die generierte ID schon existiert und ggf. Suffix hinzufügen
             std::cout << "Generierte neue Encounter-ID: " << encounter_id << std::endl;
        }

        // 2. Ziel-Dateipfad erstellen
        std::filesystem::path target_file_path;
         try {
             target_file_path = std::filesystem::path(encounters_base_dir) / (encounter_id + ".json");
             target_file_path = std::filesystem::absolute(target_file_path).lexically_normal();
         } catch (const std::exception& e) {
             std::cerr << "Fehler beim Erstellen des Zielpfads für Encounter '" << encounter_id << "': " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Erstellen des Dateipfads.\"}");
         }

         // Prüfe, ob Datei schon existiert, um Statuscode 200 oder 201 zu setzen
         bool file_existed = std::filesystem::exists(target_file_path);


        // 3. Finale JSON-Struktur aufbauen (mit Anreicherung)
        json final_encounter_data;
        final_encounter_data["id"] = encounter_id;
        final_encounter_data["name"] = encounter_name;
        final_encounter_data["description"] = incoming_data.value("description", "");
        final_encounter_data["party"] = incoming_data.value("party", json::object({{"playerCount", 4}, {"averageLevel", 1}})); // Default Party
        final_encounter_data["calculatedDifficulty"] = incoming_data.value("calculatedDifficulty", "Unknown");

        // Monster anreichern
        final_encounter_data["monsters"] = json::array();
        try {
            for (const auto& monster_ref : incoming_data["monsters"]) {
                if (!monster_ref.contains("monsterId") || !monster_ref.contains("count")) {
                     std::cerr << "Warnung: Ungültiger Monstereintrag im Request übersprungen." << std::endl;
                     continue;
                }
                std::string m_id = monster_ref["monsterId"];
                int m_count = monster_ref["count"];

                json monster_full_data = load_monster_statblock(m_id);
                if (monster_full_data == nullptr) {
                    // Konnte Monster nicht laden, entscheide, wie damit umgegangen wird
                    // Option A: Fehler werfen und Speichern abbrechen
                     json error_resp;
                     error_resp["error"] = "Referenziertes Monster nicht gefunden oder konnte nicht geladen werden.";
                     error_resp["monsterId"] = m_id;
                     return crow::response(400, error_resp.dump()); // 400 Bad Request, da Input fehlerhaft
                    // Option B: Monster überspringen (führt evtl. zu inkonsistenten Encountern)
                    // std::cerr << "Warnung: Monster " << m_id << " konnte nicht geladen werden, wird im Encounter übersprungen." << std::endl;
                    // continue;
                }

                // Benötigte Felder extrahieren
                int ac = monster_full_data.value("AC", 10);
                double cr = monster_full_data.value("CR", 0.0); // CR kann double sein (1/8 etc.)

                // HP berechnen (Durchschnitt)
                json hp_obj = monster_full_data.value("HP", json::object({{"HD", 1}, {"type", 8}, {"modifier", 0}}));
                int hd = hp_obj.value("HD", 1);
                int type = hp_obj.value("type", 8);
                int modifier = hp_obj.value("modifier", 0);
                int avg_hp = std::max(1, static_cast<int>(std::floor(hd * (static_cast<double>(type) / 2.0 + 0.5)) + modifier)); // Durchschnitt berechnen

                // Initiative Bonus berechnen
                json stats_obj = monster_full_data.value("stats", json::object({{"DEX", 10}}));
                int dex = stats_obj.value("DEX", 10);
                int init_bonus = static_cast<int>(std::floor((dex - 10) / 2.0));

                // Angereicherten Eintrag erstellen
                json enriched_monster;
                enriched_monster["monsterId"] = m_id;
                enriched_monster["count"] = m_count;
                enriched_monster["name"] = monster_full_data.value("name", m_id); // Name aus Statblock
                enriched_monster["AC"] = ac;
                enriched_monster["CR"] = cr;
                enriched_monster["averageHp"] = avg_hp;
                enriched_monster["initiativeBonus"] = init_bonus;
                // Füge keine vollen Statblocks hinzu! Nur die extrahierten Werte.

                final_encounter_data["monsters"].push_back(enriched_monster);
            }
        } catch (const std::exception& e) {
             std::cerr << "Fehler beim Anreichern der Monsterdaten: " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Verarbeiten der Monsterdaten.\"}");
        }


        // 4. In Datei schreiben
        try {
            std::ofstream output_file(target_file_path);
            if (!output_file.is_open()) {
                 std::cerr << "Fehler: Zieldatei konnte nicht zum Schreiben geöffnet werden: " << target_file_path << std::endl;
                 return crow::response(500, "{\"error\": \"Encounter konnte nicht gespeichert werden (Dateizugriff).\"}");
            }
            // Schreibe JSON pretty-printed (Einrückung 4 Leerzeichen)
            output_file << final_encounter_data.dump(4);
            output_file.close(); // Schließen, um sicherzustellen, dass alles geschrieben wurde
             std::cout << "Encounter erfolgreich gespeichert: " << target_file_path << std::endl;

        } catch (const std::exception& e) {
             std::cerr << "Fehler beim Schreiben der Encounter-Datei " << target_file_path << ": " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Speichern des Encounters.\"}");
        }

        // 5. Erfolgsantwort senden
        int status_code = file_existed ? 200 : 201; // OK oder Created
        crow::response res(status_code, final_encounter_data.dump()); // Gib die gespeicherten Daten zurück
        res.set_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/monsters/<string>").methods("PUT"_method)
        ([&](const crow::request& req, const std::string& monster_id_from_url){ // Capture base_dirs
        json incoming_data;
        try {
            incoming_data = json::parse(req.body);
        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen des Request Body (PUT Monster): " << e.what() << std::endl;
            return crow::response(400, "{\"error\": \"Ungültiges JSON im Request Body.\"}");
        }

        // 1. Zieldateipfad erstellen
        std::filesystem::path target_file_path;
         try {
             target_file_path = std::filesystem::path(monsters_base_dir) / (monster_id_from_url + ".json");
             target_file_path = std::filesystem::absolute(target_file_path).lexically_normal();
         } catch (const std::exception& e) {
             std::cerr << "Fehler beim Erstellen des Zielpfads für Monster '" << monster_id_from_url << "': " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Erstellen des Dateipfads.\"}");
         }

        // === NEU: Prüfen, ob Datei *vor* dem Schreiben existiert ===
        bool file_existed = false;
        try {
             file_existed = std::filesystem::exists(target_file_path) && std::filesystem::is_regular_file(target_file_path);
        } catch(const std::exception& e) {
             // Fehler bei exists kann passieren (z.B. Berechtigungen)
             std::cerr << "Fehler beim Prüfen der Existenz von " << target_file_path << ": " << e.what() << std::endl;
             // Man könnte hier abbrechen oder vorsichtig weitermachen
        }
        // ========================================================


        // 3. (Optional) Validierung der eingehenden Daten
        if (!incoming_data.contains("name")) {
            return crow::response(400, "{\"error\": \"Fehlendes 'name'-Feld in den Monsterdaten.\"}");
        }
        // Füge hier ggf. weitere Validierungen hinzu

        // 4. Datei zum Schreiben öffnen (überschreibt/erstellt)
        try {
            std::ofstream output_file(target_file_path); // Öffnet zum Schreiben
            if (!output_file.is_open()) {
                 std::cerr << "Fehler: Zieldatei konnte nicht zum Schreiben geöffnet werden (PUT): " << target_file_path << std::endl;
                 return crow::response(500, "{\"error\": \"Monster konnte nicht gespeichert werden (Dateizugriff).\"}");
            }
            // Schreibe die *eingehenden* Daten pretty-printed
            output_file << incoming_data.dump(4);
            output_file.close();
             if (file_existed) {
                std::cout << "Monster erfolgreich aktualisiert: " << target_file_path << std::endl;
             } else {
                std::cout << "Monster erfolgreich erstellt: " << target_file_path << std::endl;
             }

        } catch (const std::exception& e) {
             std::cerr << "Fehler beim Schreiben der Monster-Datei (PUT) " << target_file_path << ": " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Fehler beim Speichern des Monsters.\"}");
        }

        // === NEU: Statuscode basierend auf Existenz vor dem Schreiben setzen ===
        int status_code = file_existed ? 200 : 201; // 200 OK für Update, 201 Created für Neu
        // ====================================================================

        // 5. Erfolgsantwort senden
        crow::response res(status_code, incoming_data.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/monsters/<string>").methods("GET"_method)
        ([&](const crow::request& /*req*/, const std::string& monster_id){ // Capture base_dir
        std::filesystem::path monster_file_path;
        try {
            monster_file_path = std::filesystem::path(monsters_base_dir) / (monster_id + ".json");
            monster_file_path = std::filesystem::absolute(monster_file_path).lexically_normal();

            if (!std::filesystem::exists(monster_file_path) || !std::filesystem::is_regular_file(monster_file_path)) {
                std::cerr << "GET /api/monsters: Monster nicht gefunden: " << monster_file_path << std::endl;
                return crow::response(404, "{\"error\": \"Monster not found.\"}");
            }

            std::ifstream monster_file(monster_file_path);
            if (!monster_file.is_open()) {
                 std::cerr << "GET /api/monsters: Datei nicht lesbar: " << monster_file_path << std::endl;
                 return crow::response(500, "{\"error\": \"Could not read monster file.\"}");
            }

            // Lese den gesamten Dateiinhalt
            std::stringstream buffer;
            buffer << monster_file.rdbuf();
            std::string monster_content = buffer.str();

            // Sende den rohen JSON-Inhalt zurück
            crow::response res(200, monster_content);
            res.set_header("Content-Type", "application/json");
            return res;

        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Laden von Monster " << monster_id << " (" << monster_file_path << "): " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error loading monster details.\"}");
        }
    });


    // --- DELETE /api/encounters/{encounterId} (NEU) ---
     CROW_ROUTE(app, "/api/encounters/<string>").methods("DELETE"_method)
        ([&](const std::string& encounter_id) { // Capture base_dir
         std::filesystem::path file_path;
         try {
             file_path = std::filesystem::path(encounters_base_dir) / (encounter_id + ".json");
             file_path = std::filesystem::absolute(file_path).lexically_normal();

             if (!std::filesystem::exists(file_path)) {
                 return crow::response(404, "{\"error\": \"Encounter nicht gefunden.\"}");
             }
              if (!std::filesystem::is_regular_file(file_path)) {
                 return crow::response(400, "{\"error\": \"Angegebene ID gehört nicht zu einer Datei.\"}");
             }

             // Versuche zu löschen
             if (std::filesystem::remove(file_path)) {
                 std::cout << "Encounter gelöscht: " << file_path << std::endl;
                 return crow::response(204); // No Content - Standard für erfolgreiches DELETE
             } else {
                  std::cerr << "Fehler: Encounter-Datei konnte nicht gelöscht werden (Berechtigungen?): " << file_path << std::endl;
                  return crow::response(500, "{\"error\": \"Datei konnte nicht gelöscht werden.\"}");
             }

         } catch (const std::exception& e) {
             std::cerr << "Fehler beim Löschen des Encounters '" << encounter_id << "' (" << file_path << "): " << e.what() << std::endl;
             return crow::response(500, "{\"error\": \"Interner Serverfehler beim Löschen.\"}");
         }
     });


    // --- Server Start ---
    app.port(8080).multithreaded().run();
    std::cout << "Server wird beendet." << std::endl;
    return 0;
}