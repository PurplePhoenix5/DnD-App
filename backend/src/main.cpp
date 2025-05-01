#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // Für std::filesystem
#include <fstream>    // Für std::ifstream/ofstream
#include <sstream>    // Für std::stringstream
#include <exception>  // Für std::exception
#include <cmath>      // Für std::floor
#include <iomanip>    // Für std::setw (im dump für pretty print)
#include <algorithm>  // Für std::transform, std::replace_if
#include <cctype>     // Für ::tolower, ::isalnum

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

// --- Konstanten für Verzeichnisse ---
const std::string encounters_base_dir = "../data/encounters";
const std::string monsters_base_dir = "../data/monsters";
const std::string dnddata_base_dir = "../data/DnDData";
const std::string templates_base_dir = "../data/templates";
const std::vector<std::string> valid_template_types = {"trait", "attackRoll", "savingThrow", "other"}; // Liste der erlaubten Template-Typen

// --- Simpler In-Memory Cache für DnDData ---
std::map<std::string, json> dndDataCache;
// --- Ende Cache ---

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

    // Erstelle ID aus Name (+ Uses für Trait, oder andere Kriterien für Actions?)
    // Für Actions nehmen wir einfach Name und Typ als Basis für die ID
    std::string base_id = incoming_data["name"];
    if (type == "trait" && incoming_data.contains("limitedUse") && incoming_data["limitedUse"].is_object() && incoming_data["limitedUse"].contains("count")) {
        base_id += "_uses" + std::to_string(incoming_data["limitedUse"]["count"].get<int>());
    } else if (type == "savingThrow" && incoming_data.contains("saveStat") && incoming_data["saveStat"].is_string()) {
        base_id += "_" + incoming_data["saveStat"].get<std::string>();
    }
    // Fügen Sie hier Logik für andere Action-Typen hinzu, falls nötig, um IDs eindeutiger zu machen

    // Sanitisiere ID
    std::string template_id = base_id;
    std::transform(template_id.begin(), template_id.end(), template_id.begin(), ::tolower);
    std::replace_if(template_id.begin(), template_id.end(), [](char c){ return !std::isalnum(c) && c != '_'; }, '_'); // Erlaubt Unterstrich
    template_id.erase(std::unique(template_id.begin(), template_id.end(), [](char a, char b){ return a == '_' && b == '_'; }), template_id.end()); // Entfernt doppelte Unterstriche
    template_id.erase(0, template_id.find_first_not_of('_')); // Entfernt führende Unterstriche
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
        output_file << incoming_data.dump(4); // Schreibe die empfangenen Daten
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
             // Prüfe hier nicht auf is_regular_file, um eine spezifischere 404 Meldung zu geben,
             // wenn der Pfad existiert, aber kein File ist.
             throw std::runtime_error("Template not found for deletion."); // Eigene Meldung für 404
         }
         if (!std::filesystem::is_regular_file(file_path)) {
              throw std::runtime_error("Path exists but is not a regular file."); // Eigene Meldung für 404
         }

         if (std::filesystem::remove(file_path)) {
             // Erfolg, 204 No Content
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

// --- Hilfsfunktion zum Laden eines Monster-Statblocks (kann bleiben) ---
// Gibt ein leeres JSON-Objekt zurück, wenn Laden fehlschlägt oder Felder fehlen
json load_monster_statblock(const std::string& monster_id) {
    // ... (bestehende Implementierung) ...
     const std::string monsters_base_dir = "../data/monsters"; // Hinweis: base_dir ist im main, hier als Konstante kopiert

    std::filesystem::path monster_file_path;

    try {
        // Suche in beiden Ordnern
        std::vector<std::string> potential_subdirs = {"completed", "uncompleted"};
        bool file_found = false;
        for(const auto& subdir : potential_subdirs) {
             std::filesystem::path current_path;
             try {
                current_path = std::filesystem::absolute(std::filesystem::path(monsters_base_dir) / subdir / (monster_id + ".json")).lexically_normal();
                if (std::filesystem::exists(current_path) && std::filesystem::is_regular_file(current_path)) {
                    monster_file_path = current_path;
                    file_found = true;
                    break;
                }
             } catch(const std::exception& e) {
                  std::cerr << "Fehler beim Suchen nach Monster " << monster_id << " in " << subdir << ": " << e.what() << std::endl;
             }
        }

         if (!file_found) {
             std::cerr << "Monster " << monster_id << " nicht in completed oder uncompleted gefunden." << std::endl;
             return nullptr; // Signalisiert Fehler (oder 404)
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
// --- Ende Hilfsfunktion Monster Laden ---


int main() {
    crow::App<CorsMiddleware> app;
    // Konstanten für Verzeichnisse sind jetzt global definiert

    // --- Simpler In-Memory Cache für DnDData (kann bleiben) ---
    // std::map<std::string, json> dndDataCache ist global

    // --- GET /api/status (kann bleiben) ---
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
        // ... (bestehende Implementierung) ...
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
        // ... (bestehende Implementierung) ...
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

     // --- GET /api/monsters/summary (kann bleiben) ---
     CROW_ROUTE(app, "/api/monsters/summary")([&]() {
        // ... (bestehende Implementierung) ...
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

    // --- GET /api/spells (kann bleiben) ---
    CROW_ROUTE(app, "/api/spells")([&]() {
        // ... (bestehende Implementierung) ...
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

    // --- GET /api/dnddata/{filename} (kann bleiben) ---
    CROW_ROUTE(app, "/api/dnddata/<string>")
        ([&](const std::string& requested_filename) {
        // ... (bestehende Implementierung) ...
         if (requested_filename.empty()) { return crow::response(400, "{\"error\": \"Filename is required.\"}"); }
        if (requested_filename.find("..") != std::string::npos) { return crow::response(400, "{\"error\": \"Invalid filename.\"islava"); } // Geändert: isValid statt islava
        if (requested_filename.length() <= 5 || requested_filename.substr(requested_filename.length() - 5) != ".json") { return crow::response(400, "{\"error\": \"Filename must end with .json\"}"); }


        const std::string filename = requested_filename;

        if (dndDataCache.count(filename)) {
            crow::response res(dndDataCache[filename].dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "Cache");
            return res;
        }

        if (loadingPromises.count(filename)) { // Prüfe map mit .count() statt struct mit .find()
             // Wenn ein Promise läuft, gib 202 Accepted oder warte kurz (Warten komplex)
             // Einfacher: Frontend soll mit Ladeanzeige warten
             return crow::response(202, "{\"status\": \"Loading\", \"message\": \"Data is currently being loaded.\"}");
        }


        std::filesystem::path file_path;
        try {
            file_path = std::filesystem::path(dnddata_base_dir) / filename;
            file_path = std::filesystem::absolute(file_path).lexically_normal();

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
            data_file >> data_content;

            dndDataCache[filename] = data_content; // Speichere im Cache

            crow::response res(data_content.dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("X-Data-Source", "File");
            return res;

        } catch (const json::parse_error& e) {
            std::cerr << "Fehler beim Parsen der DnDData-Datei " << file_path << ": " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Error reading DnD data content.\"}");
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Laden der DnDData-Datei " << filename << " (" << file_path << "): " << e.what() << std::endl;
            return crow::response(500, "{\"error\": \"Internal server error loading DnD data.\"}");
        }
    });


    // --- NEUE Routen für ALLE Template-Typen (Trait, AttackRoll, SavingThrow, Other) ---

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
            if (std::string(e.what()).find("Template not found") != std::string::npos) {
                 return crow::response(404, "{\"error\": \"" + std::string(e.what()) + "\"}");
            }
             if (std::string(e.what()).find("Invalid characters") != std::string::npos) {
                 return crow::response(400, "{\"error\": \"" + std::string(e.what()) + "\"}");
            }
            return crow::response(500, "{\"error\": \"" + std::string(e.what()) + "\"}");
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
             if (std::string(e.what()).find("Missing or empty") != std::string::npos) {
                 return crow::response(400, "{\"error\": \"" + std::string(e.what()) + "\"}");
             }
             if (std::string(e.what()).find("exists") != std::string::npos) {
                  return crow::response(409, "{\"error\": \"" + std::string(e.what()) + "\"}"); // 409 Conflict
             }
              // Fehler beim Speichern (Dateizugriff etc.)
              return crow::response(500, "{\"error\": \"" + std::string(e.what()) + "\"}");

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
             if (std::string(e.what()).find("Template not found") != std::string::npos || std::string(e.what()).find("Path exists but is not a regular file") != std::string::npos) {
                  return crow::response(404, "{\"error\": \"" + std::string(e.what()) + "\"}");
             }
             if (std::string(e.what()).find("Invalid characters") != std::string::npos) {
                 return crow::response(400, "{\"error\": \"" + std::string(e.what()) + "\"}");
            }
              // Fehler beim Löschen (Dateizugriff etc.)
              return crow::response(500, "{\"error\": \"" + std::string(e.what()) + "\"}");

         } catch (const std::exception& e) {
             return crow::response(500, "{\"error\": \"Internal server error: " + std::string(e.what()) + "\"}");
         }
     });

    // --- RESTLICHE MONSTER/ENCOUNTER Routen (können bleiben) ---

    CROW_ROUTE(app, "/api/monsters/<string>").methods("PUT"_method)
        ([&](const crow::request& req, const std::string& monster_id_from_url){
            // ... (bestehende Implementierung) ...
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