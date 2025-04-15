# DnD-App
```
DnD-App/
├── .devcontainer/          # Konfiguration für VS Code Remote Containers
│   ├── devcontainer.json   # Hauptkonfigurationsdatei
│   └── Dockerfile          # Definition des Entwicklungs-Containers
│
├── backend/                # Alles für die C++ Logik
│   ├── src/                # C++ Quellcode
│   │   └── main.cpp        # Dein Haupteinstiegspunkt für den Server
│   │   # Hier kommen weitere .cpp/.h Dateien für Logik (Combat, Encounter etc.) hin
│   ├── include/            # Header-Dateien (optional, für größere Projekte)
│   ├── data/               # Deine Daten-Dateien
│   │   ├── monsters/       # JSON-Dateien für Monster
│   │   │   └── goblin.json
│   │   │   └── skeleton.json
│   │   │   └── ...
│   │   ├── rules/          # Markdown-Dateien für Regeln
│   │   │   └── grappling.md
│   │   │   └── ...
│   │   └── maps/           # Karten-Assets (Bilder, evtl. Pin-Daten als JSON/MD)
│   │       └── region_map.png
│   │       └── pins_region_map.json # Beispiel für separate Pin-Daten
│   ├── external/           # Externe Bibliotheken (optional, z.B. Header-only)
│   │   └── nlohmann/
│   │       └── json.hpp    # Wenn du nlohmann/json direkt einbindest
│   └── CMakeLists.txt      # Build-Konfiguration für das C++ Backend
│
├── frontend/               # Alles für die Web-Oberfläche (UI)
│   ├── public/             # Statische Assets (index.html, Favicon etc.)
│   │   └── index.html
│   ├── src/                # Frontend Quellcode (JavaScript/TypeScript, CSS, etc.)
│   │   ├── components/     # Wiederverwendbare UI-Teile (z.B. CombatantCard.jsx)
│   │   ├── views/          # Hauptansichten/Tabs (z.B. CombatTrackerView.jsx)
│   │   └── App.jsx         # Haupt-App-Komponente (Beispiel für React)
│   │   └── main.jsx        # Einstiegspunkt fürs Frontend (Beispiel für React/Vite)
│   ├── package.json        # Node.js Abhängigkeiten und Skripte
│   └── # Weitere Frontend Konfigurationsdateien (vite.config.js, etc.)
│
├── .gitignore              # Definiert, welche Dateien *nicht* zu Git hinzugefügt werden
└── README.md               # Beschreibung deines Projekts
```