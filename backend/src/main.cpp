#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // Für std::filesystem
#include <fstream>    // Für std::ifstream/ofstream
#include <sstream>    // Für std::stringstream
#include <exception>  // Für std::exception
#include <cmath>      // Für std::floor
#include <iomanip>    // Für std::setw (im dump für pretty print)
#include <algorithm>  // Für std::transform, std::replace_if, std::find, std::unique
#include <cctype>     // Für ::tolower, ::isalnum
#include <set>        // Für std::set (zum Verfolgen laufender Ladevorgänge)
#include <map>        // Für Benutzerdaten

// Crow Header
#include "crow.h"
// nlohmann/json Header
#include "nlohmann/json.hpp"

// Deine CorsMiddleware (wie gehabt)
struct CorsMiddleware {
     struct context {};
    void before_handle(crow::request& req, crow::response& res, context& /*ctx*/) {

        if (req.method == "OPTIONS"_method) {
            res.add_header("Access-Control-Allow-Origin", "http://localhost:5173");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.code = 204;
            res.end();
        }
    }
    void after_handle(crow::request& /*req*/, crow::response& res, context& /*ctx*/) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:5173");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};

using json = nlohmann::json;

const std::string user_data_file = "../data/users.json"; // Pfad zur Benutzerdatei

// --- Simpler In-Memory Benutzerdaten-Speicher ---
// WICHTIG: Dies ist NUR für Demo-Zwecke. Passwörter NIEMALS so speichern!
// In einer echten Anwendung: Passwörter hashen & salzen, Datenbank verwenden.
std::map<std::string, json> users_db;

// Funktion zum Laden der Benutzerdaten aus einer Datei
void load_users() {
    try {
        std::filesystem::path user_file_path = std::filesystem::absolute(user_data_file).lexically_normal();
        if (std::filesystem::exists(user_file_path) && std::filesystem::is_regular_file(user_file_path)) {
            std::ifstream user_file(user_file_path);
            if (user_file.is_open()) {
                json user_data_json;
                user_file >> user_data_json;
                if (user_data_json.is_object()) {
                    users_db = user_data_json.get<std::map<std::string, json>>();
                    std::cout << "Benutzerdaten geladen aus " << user_data_file << std::endl;
                } else {
                     std::cerr << "Warnung: users.json enthält kein gültiges JSON-Objekt." << std::endl;
                }
            } else {
                std::cerr << "Warnung: Konnte users.json nicht öffnen." << std::endl;
            }
        } else {
             std::cerr << "Warnung: users.json nicht gefunden. Verwende leere Benutzerdatenbank." << std::endl;
             // Optional: Standardbenutzer erstellen, wenn keine Datei da ist
             // users_db["dm"] = {{"password", "dm_password"}, {"role", "DM"}};
             // users_db["player"] = {{"password", "player_password"}, {"role", "Player"}};
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Laden der Benutzerdaten: " << e.what() << std::endl;
    }
}
// --- Ende Benutzerdaten ---

// --- Globale Konstanten und Caches ---
const std::string encounters_base_dir = "../data/encounters";
const std::string monsters_base_dir = "../data/monsters";
const std::string dnddata_base_dir = "../data/DnDData";
const std::string templates_base_dir = "../data/templates";
const std::vector<std::string> valid_template_types = {"trait", "attackRoll", "savingThrow", "other"}; // Liste der erlaubten Template-Typen

// Simpler In-Memory Cache für DnDData
std::map<std::string, json> dndDataCache;
// Set zum Verfolgen, welche DnDData Dateien gerade geladen werden
std::set<std::string> currently_loading_dnd_data;
// --- Ende Globale Konstanten und Caches ---


// --- Hilfsfunktionen für Template-Handling ---

// Überprüft und normalisiert einen Template-Typ-String
bool is_valid_template_type(const std::string& type) {
    return std::find(valid_template_types.begin(), valid_template_types.end(), type) != valid_template_types.end();
}

// Generiert einen sicheren Dateipfad für ein Template
std::filesystem::path get_template_filepath(const std::string& type, const std::string& id) {
    // Grundlegende ID-Validierung
    if (id.empty() || id.find("..") != std::string::npos || id.find('/') != std::string::npos || id.find('\\') != std::string::npos) {
        throw std::runtime_error("Invalid characters in template ID.");
    }
     if (!is_valid_template_type(type)) {
         throw std::runtime_error("Invalid template type.");
     }

    std::filesystem::path file_path = std::filesystem::path(templates_base_dir) / type / (id + ".json");
    return std::filesystem::absolute(file_path).lexically_normal();
}

// Listet Templates eines Typs auf (gibt nur ID und Name zurück)
json list_templates_by_type(const std::string& type) {
    if (!is_valid_template_type(type)) {
         return json::array(); // Ungültiger Typ -> leeres Array
     }
    json template_list = json::array();
    std::filesystem::path template_dir = std::filesystem::path(templates_base_dir) / type;

    try {
        // Stelle sicher, dass das Verzeichnis existiert
        if (!std::filesystem::exists(template_dir) || !std::filesystem::is_directory(template_dir)) {
            std::filesystem::create_directories(template_dir); // Erstelle es, wenn es fehlt
            return template_list; // Gib leere Liste zurück, wenn es gerade erst erstellt wurde
        }

        for (const auto& entry : std::filesystem::directory_iterator(template_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    try {
                        json data;
                        file >> data;
                        json item;
                        item["id"] = entry.path().stem().string();
                        // Sicherer Zugriff auf den Namen, Fallback wenn fehlt
                        item["name"] = data.value("name", "Unknown Template");
                        template_list.push_back(item);
                    } catch (const std::exception& e) {
                        std::cerr << "Fehler beim Verarbeiten von Template " << entry.path() << ": " << e.what() << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Auflisten von Templates (" << type << "): " << e.what() << std::endl;
        // Fehler beim Auflisten sollte 500 sein, wird vom Handler behandelt
        throw; // Exception weiterwerfen
    }
    return template_list;
}

// Lädt ein spezifisches Template
json get_template_by_type(const std::string& type, const std::string& id) {
    try {
         std::filesystem::path file_path = get_template_filepath(type, id);

         if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
            throw std::runtime_error("Template not found."); // Eigene Meldung für 404
         }
          std::ifstream file(file_path);
         if (!file.is_open()) { throw std::runtime_error("Could not open template file."); }

         json template_data;
         file >> template_data;
         return template_data;

    } catch (const json::parse_error& e) {
        std::cerr << "Fehler beim Parsen von Template " << type << "/" << id << ": " << e.what() << std::endl;
        throw std::runtime_error("Error reading template content."); // Eigene Meldung für 500
    } catch (const std::runtime_error& e) {
        // Propagiert Fehler von get_template_filepath oder "Template not found"
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Laden von Template " << type << "/" << id << ": " << e.what() << std::endl;
        throw std::runtime_error("Internal server error loading template."); // Generische 500 Meldung
    }
}

// Speichert ein Template (generiert ID aus Inhalt)
json save_template_by_type(const std::string& type, const json& incoming_data) {
    if (!is_valid_template_type(type)) {
        throw std::runtime_error("Invalid template type for saving.");
    }

     // Validierung: Braucht mindestens 'name'
     if (!incoming_data.contains("name") || !incoming_data["name"].is_string() || incoming_data["name"].get<std::string>().empty()) {
         throw std::runtime_error("Missing or empty 'name' field for template.");
     }

    // Erstelle ID aus Name (+ Zusätze basierend auf Typ für Eindeutigkeit)
    std::string base_id_name = incoming_data["name"].get<std::string>();
    std::string template_id = base_id_name;

    if (type == "trait") {
        if (incoming_data.contains("limitedUse") && incoming_data["limitedUse"].is_object() && incoming_data["limitedUse"].contains("count") && incoming_data["limitedUse"]["count"].is_number_integer() && incoming_data["limitedUse"]["count"].get<int>() > 0) {
             template_id += "_uses" + std::to_string(incoming_data["limitedUse"]["count"].get<int>());
        }
    } else if (type == "savingThrow") {
        if (incoming_data.contains("saveStat") && incoming_data["saveStat"].is_string() && !incoming_data["saveStat"].get<std::string>().empty()) {
             template_id += "_" + incoming_data["saveStat"].get<std::string>();
        }
    }
    // Fügen Sie hier Logik für andere Action-Typen hinzu, falls nötig, um IDs eindeutiger zu machen
    // Z.B. für Attack Roll: rangeType? firstDamageType?

    // Sanitisiere ID
    std::transform(template_id.begin(), template_id.end(), template_id.begin(), ::tolower);
    template_id.erase(std::remove_if(template_id.begin(), template_id.end(), [](char c){ return !std::isalnum(c) && c != '_'; }), template_id.end()); // Entfernt nicht-alphanumerische Zeichen außer Unterstrich
    template_id.erase(std::unique(template_id.begin(), template_id.end(), [](char a, char b){ return a == '_' && b == '_'; }), template_id.end()); // Entfernt doppelte Unterstriche
    template_id.erase(0, template_id.find_first_not_of('_')); // Entfernt führende Unterstriche
    if (template_id.empty()) template_id = "template"; // Fallback falls Name nur aus Sonderzeichen bestand
    template_id.erase(template_id.find_last_not_of('_') + 1); // Entfernt nachgestellte Unterstriche


    std::filesystem::path file_path;
    try {
        file_path = get_template_filepath(type, template_id); // Nutze generierte ID im Pfad

        // Prüfen, ob Template mit dieser ID schon existiert
        if (std::filesystem::exists(file_path)) {
            throw std::runtime_error("A template with this ID already exists."); // Eigene Meldung für 409
        }

        // Stelle sicher, dass das Verzeichnis existiert
        std::filesystem::create_directories(file_path.parent_path());

        std::ofstream output_file(file_path);
        if (!output_file.is_open()) { throw std::runtime_error("Could not save template file."); }
        output_file << incoming_data.dump(4); // Schreibe die empfangenen Daten (mit 4 Spaces Einrückung)
        output_file.close();

        json response_data = incoming_data;
        response_data["id"] = template_id; // Füge die generierte ID zur Antwort hinzu
        return response_data; // Rückgabe der gespeicherten Daten + ID

    } catch (const std::runtime_error& e) {
         // Propagiert Fehler von get_template_filepath oder "Template already exists" oder "Could not save"
        throw;
    }
    catch (const std::exception& e) {
        std::cerr << "Fehler beim Speichern von Template (" << type << ", ID: " << template_id << "): " << e.what() << std::endl;
        throw std::runtime_error("Internal server error saving template.");
    }
}

// Löscht ein spezifisches Template
void delete_template_by_type(const std::string& type, const std::string& id) {
    try {
         std::filesystem::path file_path = get_template_filepath(type, id);

         if (!std::filesystem::exists(file_path)) {
             throw std::runtime_error("Template not found for deletion."); // Eigene Meldung für 404
         }
         if (!std::filesystem::is_regular_file(file_path)) {
              throw std::runtime_error("Path exists but is not a regular file."); // Eigene Meldung für 404
         }

         if (std::filesystem::remove(file_path)) {
             // Erfolg, 204 No Content wird im Handler gesendet
         } else {
              throw std::runtime_error("Could not delete template file."); // Eigene Meldung für 500
         }
     } catch (const std::runtime_error& e) {
         // Propagiert Fehler von get_template_filepath oder "Template not found" oder "Could not delete"
        throw;
     }
    catch (const std::exception& e) {
        std::cerr << "Fehler beim Löschen von Template (" << type << ", ID: " << id << "): " << e.what() << std::endl;
        throw std::runtime_error("Internal server error deleting template.");
    }
}

// --- Hilfsfunktion zum Laden eines Monster-Statblocks (geringfügig angepasst) ---
// Sucht jetzt in completed und uncompleted Ordnern
json load_monster_statblock(const std::string& monster_id) {
     std::filesystem::path monster_file_path;
     bool file_found = false;

     try {
         std::vector<std::string> potential_subdirs = {"completed", "uncompleted"};
         for(const auto& subdir : potential_subdirs) {
              std::filesystem::path current_path;
              try {
                 current_path = std::filesystem::absolute(std::filesystem::path(monsters_base_dir) / subdir / (monster_id + ".json")).lexically_normal();
                 if (std::filesystem::exists(current_path) && std::filesystem::is_regular_file(current_path)) {
                     monster_file_path = current_path;
                     file_found = true;
                     break; // Datei gefunden, Schleife beenden
                 }
              } catch(const std::exception& e) {
                   std::cerr << "Fehler beim Suchen nach Monster " << monster_id << " in " << subdir << ": " << e.what() << std::endl;
              }
         }

          if (!file_found) {
              std::cerr << "Monster " << monster_id << " nicht in completed oder uncompleted gefunden." << std::endl;
              return nullptr; // Signalisiert 404
          }

         std::ifstream monster_file(monster_file_path);
         if (!monster_file.is_open()) {
             std::cerr << "Fehler: Monster-Datei konnte nicht geöffnet werden: " << monster_file_path << std::endl;
              return nullptr; // Signalisiert 500
         }

         json monster_data;
         monster_file >> monster_data; // Parse JSON
         return monster_data;

     } catch (const json::parse_error& e) {
         std::cerr << "Fehler beim Parsen der Monster-JSON-Datei " << monster_file_path << ": " << e.what() << std::endl;
         return nullptr; // Signalisiert 500
     } catch (const std::exception& e) {
         std::cerr << "Fehler beim Laden der Monster-Datei " << monster_file_path << ": " << e.what() << std::endl;
         return nullptr; // Signalisiert 500
     }
}
// --- Ende Hilfsfunktion Monster Laden ---


int main() {
    crow::App<CorsMiddleware> app;
    // Globale Konstanten und Caches sind bereits deklariert

    load_users();

    // --- NEUE Route: POST /api/login ---
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([&](const crow::request& req) {
        json request_body;
        try {
            request_body = json::parse(req.body);
        } catch (const json::parse_error& e) {
            return crow::response(400, "{\"error\": \"Invalid JSON body.\"}");
        }

        if (!request_body.contains("username") || !request_body["username"].is_string() ||
            !request_body.contains("password") || !request_body["password"].is_string()) {
            return crow::response(400, "{\"error\": \"Missing username or password.\"}");
        }

        std::string username = request_body["username"];
        std::string password = request_body["password"];

        // Suche Benutzer (Groß-/Kleinschreibung beachten beim Key der Map)
        if (users_db.count(username)) {
            const json& user_data = users_db[username];
            // === UNSICHERER PASSWORTVERGLEICH (NUR FÜR DEMO) ===
            if (user_data.contains("password") && user_data["password"] == password) {
                // Erfolgreich eingeloggt
                json response_data;
                response_data["username"] = username;
                response_data["role"] = user_data.value("role", "Player"); // Default "Player", falls Rolle fehlt
                // Hier könnte man einen Session Token/JWT generieren und zurückgeben
                std::cout << "Login erfolgreich für Benutzer: " << username << " mit Rolle: " << response_data["role"] << std::endl;
                return crow::response(200, response_data.dump());
            }
            // === ENDE UNSICHERER VERGLEICH ===
        }

        // Benutzer nicht gefunden oder Passwort falsch
        std::cerr << "Login fehlgeschlagen für Benutzer: " << username << std::endl;
        return crow::response(401, "{\"error\": \"Invalid username or password.\"}"); // 401 Unauthorized
    });

    CROW_ROUTE(app, "/api/users/list").methods("GET"_method)
    ([&]() {
        json username_list = json::array();
        // Iteriere über die geladenen Benutzerdaten (users_db ist global)
        for(const auto& pair : users_db) {
            username_list.push_back(pair.first); // Füge nur den Key (Username) hinzu
        }
        // Optional: Sortiere die Liste alphabetisch
        // std::sort(username_list.begin(), username_list.end()); // Geht nicht direkt mit nlohmann::json array

        // Wenn Sortierung benötigt wird, erst in std::vector umwandeln:
        std::vector<std::string> sorted_usernames;
        for(const auto& username_json : username_list) {
             if (username_json.is_string()) {
                 sorted_usernames.push_back(username_json.get<std::string>());
             }
        }
        std::sort(sorted_usernames.begin(), sorted_usernames.end());

        // Konvertiere sortierten Vektor zurück in JSON Array
        json sorted_username_list = json::parse(json(sorted_usernames).dump()); // Konvertiere Vektor zu JSON


        crow::response res(sorted_username_list.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // --- GET /api/status ---
    CROW_ROUTE(app, "/api/status")([]() {
        json response;
        response["status"] = "OK";
        response["message"] = "DnD Backend ist bereit!";
        crow::response res(response.dump());
        res.set_header("Content-Type", "application/json");
        return res;
     });

    // --- GET /api/encounters ---
    CROW_ROUTE(app, "/api/encounters")([&]() {
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
     CROW_ROUTE(app, "/api/monsters/summary")([&]() {
        json monster_summary_list = json::array();
        try {
             for (const auto& entry : std::filesystem::recursive_directory_iterator(monsters_base_dir)) {
                 if (entry.is_regular_file() && entry.path().extension() == ".json") {
                     std::ifstream file(entry.path());
                     if (file.is_open()) {
                         try {
                             json data;
                             file >> data;
                             json summary_item;
                             summary_item["id"] = entry.path().stem().string();
                             summary_item["name"] = data.value("basics", json::object()).value("name", "Unknown");
                             summary_item["cr"] = data.value("basics", json::object()).value("CR", 0.0);
                             summary_item["size"] = data.value("basics", json::object()).value("size", "Medium");
                             summary_item["type"] = data.value("basics", json::object()).value("type", "unknown");
                             summary_item["complete"] = data.value("complete", false);
                             monster_summary_list.push_back(summary_item);
                         } catch (const std::exception& e) {
                             std::cerr << "Fehler beim Verarbeiten der Monster-Datei " << entry.path() << ": " << e.what() << std::endl;
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
    CROW_ROUTE(app, "/api/spells")([&]() {
         const std::string spells_file_path_str = "../data/spells/spells.json";
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

            std::stringstream buffer;
            buffer << spells_file.rdbuf();
            std::string spells_content = buffer.str();

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
        ([&](const std::string& requested_filename) {
        // === VALIDIERUNG ===
        if (requested_filename.empty()) { return crow::response(400, "{\"error\": \"Filename is required.\"}"); }
        if (requested_filename.find("..") != std::string::npos) { return crow::response(400, "{\"error\": \"Invalid characters in filename.\"}"); }
        if (requested_filename.length() <= 5 || requested_filename.substr(requested_filename.length() - 5) != ".json") { return crow::response(400, "{\"error\": \"Filename must end with .json\"}"); }

        const std::string filename = requested_filename;

        // --- Cache Check ---
        if (dndDataCache.count(filename)) {
            crow::response res(dndDataCache[filename].dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "Cache");
            return res;
        }

        // --- Check if loading ---
        if (currently_loading_dnd_data.count(filename)) {
             // Data is already being loaded, inform client (202 Accepted)
             return crow::response(202, "{\"status\": \"Loading\", \"message\": \"Data is currently being loaded.\"}");
        }

        // --- Start Loading ---
        currently_loading_dnd_data.insert(filename); // Markiere als laufenden Ladevorgang

        std::filesystem::path file_path;
        try {
            file_path = std::filesystem::path(dnddata_base_dir) / filename;
            file_path = std::filesystem::absolute(file_path).lexically_normal();

            if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
                std::cerr << "Fehler: DnDData-Datei nicht gefunden: " << file_path << std::endl;
                 currently_loading_dnd_data.erase(filename); // Entferne aus Ladeliste bei Fehler
                return crow::response(404, "{\"error\": \"Requested DnD data file not found.\"}");
            }

            std::ifstream data_file(file_path);
            if (!data_file.is_open()) {
                 std::cerr << "Fehler: DnDData-Datei konnte nicht geöffnet werden: " << file_path << std::endl;
                 currently_loading_dnd_data.erase(filename); // Entferne aus Ladeliste bei Fehler
                 return crow::response(500, "{\"error\": \"Could not open DnD data file.\"}");
            }

            json data_content;
            data_file >> data_content; // Direkt als JSON parsen

            // --- Zum Cache hinzufügen ---
            dndDataCache[filename] = data_content;
            currently_loading_dnd_data.erase(filename); // Entferne aus Ladeliste bei Erfolg

            crow::response res(data_content.dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "File");
            return res;

        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen der DnDData-Datei " << file_path << ": " << e.what() << std::endl;
            currently_loading_dnd_data.erase(filename); // Entferne aus Ladeliste bei Fehler
            return crow::response(500, "{\"error\": \"Error reading DnD data content.\"}");
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Laden der DnDData-Datei " << filename << " (" << file_path << "): " << e.what() << std::endl;
            currently_loading_dnd_data.erase(filename); // Entferne aus Ladeliste bei Fehler
            return crow::response(500, "{\"error\": \"Internal server error loading DnD data.\"}");
        }
    });


    // --- Routen für ALLE Template-Typen (Trait, AttackRoll, SavingThrow, Other) ---

    // GET /api/templates/{type} (Liste aller Templates eines Typs)
    CROW_ROUTE(app, "/api/templates/<string>").methods("GET"_method)
        ([&](const std::string& type) {
        if (!is_valid_template_type(type)) {
             return crow::response(400, "{\"error\": \"Invalid template type.\"}");
         }
        try {
            json template_list = list_templates_by_type(type);
            crow::response res(template_list.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        } catch (const std::exception& e) {
            return crow::response(500, "{\"error\": \"Serverfehler beim Auflisten der Templates: " + std::string(e.what()) + "\"}");
        }
    });

    // GET /api/templates/{type}/{templateId} (Einzelnes Template holen)
    CROW_ROUTE(app, "/api/templates/<string>/<string>").methods("GET"_method)
        ([&](const std::string& type, const std::string& template_id) {
        if (!is_valid_template_type(type)) {
             return crow::response(400, "{\"error\": \"Invalid template type.\"}");
         }
        try {
             json template_data = get_template_by_type(type, template_id);
             crow::response res(template_data.dump());
             res.set_header("Content-Type", "application/json");
             return res;
        } catch (const std::runtime_error& e) {
            // Spezifischere Fehlerbehandlung basierend auf Meldung aus Helfer
            std::string error_msg = e.what();
            if (error_msg.find("Template not found") != std::string::npos || error_msg.find("Invalid characters") != std::string::npos) {
                 return crow::response(404, "{\"error\": \"" + error_msg + "\"}"); // 404 Not Found oder 400 Bad Request
            }
            // Andere Laufzeitfehler (z.B. Datei nicht lesbar, Parsefehler)
            return crow::response(500, "{\"error\": \"" + error_msg + "\"}");
        } catch (const std::exception& e) {
            return crow::response(500, "{\"error\": \"Internal server error: " + std::string(e.what()) + "\"}");
        }
    });

    // POST /api/templates/{type} (Neues Template speichern)
     CROW_ROUTE(app, "/api/templates/<string>").methods("POST"_method)
         ([&](const crow::request& req, const std::string& type) {
         if (!is_valid_template_type(type)) {
             return crow::response(400, "{\"error\": \"Invalid template type.\"}");
         }
         json incoming_data;
         try { incoming_data = json::parse(req.body); } catch (...) { return crow::response(400, "{\"error\": \"Invalid JSON body.\"}"); }

         try {
             json saved_template_data = save_template_by_type(type, incoming_data);
             crow::response res(201, saved_template_data.dump()); // 201 Created
             res.set_header("Content-Type", "application/json");
             return res;
         } catch (const std::runtime_error& e) {
             std::string error_msg = e.what();
             if (error_msg.find("Missing or empty") != std::string::npos || error_msg.find("Invalid characters") != std::string::npos) {
                 return crow::response(400, "{\"error\": \"" + error_msg + "\"}");
             }
             if (error_msg.find("exists") != std::string::npos) {
                  return crow::response(409, "{\"error\": \"" + error_msg + "\"}"); // 409 Conflict
             }
              // Andere Laufzeitfehler (z.B. Datei konnte nicht gespeichert werden)
              return crow::response(500, "{\"error\": \"" + error_msg + "\"}");

         } catch (const std::exception& e) {
             return crow::response(500, "{\"error\": \"Internal server error: " + std::string(e.what()) + "\"}");
         }
     });


    // DELETE /api/templates/{type}/{templateId}
     CROW_ROUTE(app, "/api/templates/<string>/<string>").methods("DELETE"_method)
         ([&](const std::string& type, const std::string& template_id) {
         if (!is_valid_template_type(type)) {
             return crow::response(400, "{\"error\": \"Invalid template type.\"}");
         }
         try {
             delete_template_by_type(type, template_id);
             return crow::response(204); // No Content

         } catch (const std::runtime_error& e) {
              std::string error_msg = e.what();
             if (error_msg.find("Template not found") != std::string::npos || error_msg.find("Path exists but is not a regular file") != std::string::npos || error_msg.find("Invalid characters") != std::string::npos) {
                  return crow::response(404, "{\"error\": \"" + error_msg + "\"}"); // 404 Not Found oder 400 Bad Request
             }
              // Andere Laufzeitfehler (z.B. Datei konnte nicht gelöscht werden)
              return crow::response(500, "{\"error\": \"" + error_msg + "\"}");

         } catch (const std::exception& e) {
             return crow::response(500, "{\"error\": \"Internal server error: " + std::string(e.what()) + "\"}");
         }
     });

    // --- RESTLICHE MONSTER/ENCOUNTER Routen (können bleiben) ---

    CROW_ROUTE(app, "/api/monsters/<string>").methods("PUT"_method)
        ([&](const crow::request& req, const std::string& monster_id_from_url){
             json incoming_data;
            try { incoming_data = json::parse(req.body); } catch (...) { return crow::response(400, "{\"error\": \"Ungültiges JSON im Request Body.\"}"); }

            if (!incoming_data.contains("basics") || !incoming_data["basics"].is_object()) { return crow::response(400, "{\"error\": \"Missing or invalid 'basics' object.\"}"); }
            const json& basics_data = incoming_data["basics"];
            if (!basics_data.contains("name") || !basics_data["name"].is_string() || basics_data["name"].get<std::string>().empty()) { return crow::response(400, "{\"error\": \"Missing or empty 'basics.name' field.\"}"); }
            if (!basics_data.contains("CR") || !basics_data["CR"].is_number()) { return crow::response(400, "{\"error\": \"Missing or invalid 'basics.CR' field (must be a number).\"}"); }

            bool is_complete = incoming_data.value("complete", false);
            std::string target_sub_dir = is_complete ? "completed" : "uncompleted";
            std::string opposite_sub_dir = is_complete ? "uncompleted" : "completed";

            std::filesystem::path target_dir_path = std::filesystem::path(monsters_base_dir) / target_sub_dir;
            std::filesystem::path target_file_path;
            std::filesystem::path old_file_path;

             try {
                 std::filesystem::create_directories(target_dir_path);
                 std::filesystem::create_directories(std::filesystem::path(monsters_base_dir) / opposite_sub_dir); // Stelle sicher, dass auch der andere Ordner existiert

                 target_file_path = std::filesystem::absolute(target_dir_path / (monster_id_from_url + ".json")).lexically_normal();
                 old_file_path = std::filesystem::absolute(std::filesystem::path(monsters_base_dir) / opposite_sub_dir / (monster_id_from_url + ".json")).lexically_normal();

             } catch (const std::exception& e) {
                 std::cerr << "Fehler beim Erstellen/Normalisieren der Monster-Dateipfade für '" << monster_id_from_url << "': " << e.what() << std::endl;
                 return crow::response(500, "{\"error\": \"Interner Fehler beim Erstellen des Dateipfads.\"}");
             }

             bool file_existed_in_other_dir = std::filesystem::exists(old_file_path) && std::filesystem::is_regular_file(old_file_path);
             bool file_existed_in_target_dir = std::filesystem::exists(target_file_path) && std::filesystem::is_regular_file(target_file_path);
             bool created_new = !file_existed_in_target_dir && !file_existed_in_other_dir; // Neu, wenn in keinem der beiden Ordner existierte

            try {
                std::ofstream output_file(target_file_path);
                if (!output_file.is_open()) {
                     std::cerr << "Fehler: Zieldatei konnte nicht zum Schreiben geöffnet werden: " << target_file_path << std::endl;
                     return crow::response(500, "{\"error\": \"Monster konnte nicht gespeichert werden (Dateizugriff).\"}");
                }
                output_file << incoming_data.dump(4);
                output_file.close();

                if (file_existed_in_other_dir) {
                    try {
                         if(std::filesystem::remove(old_file_path)) {
                              std::cout << "Alte Monsterdatei gelöscht: " << old_file_path << std::endl;
                         } else {
                              // Das ist eine Warnung, kein kritischer Fehler, da die neue Datei gespeichert wurde.
                              std::cerr << "Warnung: Konnte alte Monsterdatei nicht löschen: " << old_file_path << std::endl;
                         }
                    } catch(const std::exception& e) {
                         std::cerr << "Warnung: Fehler beim Löschen der alten Monsterdatei " << old_file_path << ": " << e.what() << std::endl;
                    }
                }

                std::cout << "Monster erfolgreich gespeichert/aktualisiert: " << target_file_path << std::endl;

            } catch (const std::exception& e) {
                 std::cerr << "Fehler beim Schreiben der Monster-Datei " << target_file_path << ": " << e.what() << std::endl;
                 return crow::response(500, "{\"error\": \"Interner Fehler beim Speichern des Monsters.\"}");
            }

            int status_code = created_new ? 201 : 200; // OK oder Created

            crow::response res(status_code, incoming_data.dump()); // Gib die gespeicherten Daten zurück
            res.set_header("Content-Type", "application/json");
            return res;
    });


    CROW_ROUTE(app, "/api/monsters/<string>").methods("GET"_method)
        ([&](const std::string& monster_id){
        // Diese Route nutzt jetzt load_monster_statblock, die in beiden Ordnern sucht
        json monster_data = load_monster_statblock(monster_id);

        if (monster_data == nullptr) { // load_monster_statblock gibt nullptr bei Fehler/Nicht gefunden zurück
            return crow::response(404, "{\"error\": \"Monster not found or could not be loaded.\"}");
        }

        crow::response res(monster_data.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });


    CROW_ROUTE(app, "/api/monsters/<string>").methods("DELETE"_method)
    ([&](const std::string& monster_id) {
     std::filesystem::path path_completed = std::filesystem::absolute(std::filesystem::path(monsters_base_dir) / "completed" / (monster_id + ".json")).lexically_normal();
     std::filesystem::path path_uncompleted = std::filesystem::absolute(std::filesystem::path(monsters_base_dir) / "uncompleted" / (monster_id + ".json")).lexically_normal();
     bool deleted = false;
     bool attempted_delete = false; // Flag, um zu sehen, ob überhaupt ein Löschversuch gestartet wurde

     try {
         if (std::filesystem::exists(path_completed) && std::filesystem::is_regular_file(path_completed)) {
              attempted_delete = true;
              if (std::filesystem::remove(path_completed)) {
                  deleted = true;
                  std::cout << "Monster gelöscht: " << path_completed << std::endl;
              } else {
                 std::cerr << "Fehler: Konnte Monsterdatei nicht löschen (completed): " << path_completed << std::endl;
                 // Wenn wir hier einen Fehler haben und noch nichts gelöscht wurde, senden wir 500
                 if (!deleted) return crow::response(500, "{\"error\": \"Could not delete monster file (completed).\"}");
              }
         }
          if (std::filesystem::exists(path_uncompleted) && std::filesystem::is_regular_file(path_uncompleted)) {
               attempted_delete = true;
              if (std::filesystem::remove(path_uncompleted)) {
                  deleted = true; // Setze deleted auf true, auch wenn es nur hier gefunden wurde
                  std::cout << "Monster gelöscht: " << path_uncompleted << std::endl;
              } else {
                 std::cerr << "Fehler: Konnte Monsterdatei nicht löschen (uncompleted): " << path_uncompleted << std::endl;
                 // Wenn wir hier einen Fehler haben und noch nichts gelöscht wurde, senden wir 500
                  if (!deleted) return crow::response(500, "{\"error\": \"Could not delete monster file (uncompleted).\"}");
              }
         }

         if (deleted) {
              return crow::response(204); // No Content
         } else if (attempted_delete) {
              // Wir haben versucht zu löschen (Datei existierte als Pfad) aber konnten es nicht als Datei behandeln
               return crow::response(404, "{\"error\": \"Monster file found but could not be identified as regular file for deletion.\"}");
         }
          else {
               // Weder in completed noch in uncompleted gefunden
               return crow::response(404, "{\"error\": \"Monster not found in completed or uncompleted folders.\"}");
         }

     } catch (const std::exception& e) {
         std::cerr << "Fehler beim Löschen des Monsters '" << monster_id << "': " << e.what() << std::endl;
         return crow::response(500, "{\"error\": \"Internal server error during deletion.\"}");
     }
 });

    


    // --- Server Start ---
    app.port(8080).multithreaded().run();
    std::cout << "Server wird beendet." << std::endl;
    return 0;
}