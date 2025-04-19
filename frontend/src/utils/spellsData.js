// frontend/src/utils/spellsData.js

// Globale Variable zum Speichern der geladenen Daten (einfacher Cache)
let allSpellsData = null;
// Promise, um mehrfaches Laden zu verhindern
let spellsLoadingPromise = null;

// Funktion zum Laden der Spells vom Backend
async function loadAllSpells() {
    // Wenn Daten schon geladen sind, gib sie zurück
    if (allSpellsData !== null) {
        return allSpellsData;
    }
    // Wenn schon ein Ladevorgang läuft, warte darauf
    if (spellsLoadingPromise !== null) {
        return spellsLoadingPromise;
    }

    console.log("Fetching spell data from API..."); // Log zum Debuggen

    // Starte den Ladevorgang
    spellsLoadingPromise = fetch('/api/spells') // Verwende den neuen API-Endpunkt
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error fetching spells! status: ${response.status}`);
            }
            return response.json(); // Parse die JSON-Antwort
        })
        .then(data => {
            console.log("Spell data fetched successfully.");
            allSpellsData = data; // Speichere die geparsten Daten
            spellsLoadingPromise = null; // Setze Promise zurück nach Erfolg
            return allSpellsData;
        })
        .catch(error => {
            console.error('Failed to load spell data:', error);
            allSpellsData = null; // Setze Daten zurück bei Fehler
            spellsLoadingPromise = null; // Setze Promise zurück bei Fehler
            // Optional: Fehler weiterwerfen oder null zurückgeben
            // throw error; // Oder return null;
            return null;
        });

    return spellsLoadingPromise;
}

// Hilfsfunktion, um ein einzelnes Spell-Objekt abzurufen
// Macht die Funktion async, da sie auf das Laden warten muss
export async function getSpellData(spellName) {
    // Stelle sicher, dass die Daten geladen sind (oder werden)
    const spells = await loadAllSpells();

    // Wenn Laden fehlschlug oder keine Daten da sind
    if (!spells) {
        console.warn(`Cannot get spell data for "${spellName}", spell data not loaded.`);
        return null;
    }

    // Suche nach dem Spell (Groß-/Kleinschreibung ignorieren)
    const lowerCaseName = spellName?.toLowerCase();
    if (!lowerCaseName) return null;

    // Finde den Schlüssel im geladenen Objekt
    const foundKey = Object.keys(spells).find(key => key.toLowerCase() === lowerCaseName);
    return foundKey ? spells[foundKey] : null;
}

// Funktion, um alle Spells abzurufen (falls benötigt)
// Macht die Funktion async
export async function getAllSpells() {
    return await loadAllSpells();
}

// Rufe loadAllSpells einmal beim Initialisieren des Moduls auf,
// damit die Daten im Hintergrund geladen werden.
// Komponenten, die getSpellData verwenden, werden dann über das Promise warten.
loadAllSpells();