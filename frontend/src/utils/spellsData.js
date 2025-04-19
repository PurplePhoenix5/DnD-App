// frontend/src/utils/spellsData.js

// Globale Variable zum Speichern der geladenen Daten (einfacher Cache)
let allSpellsData = null;
// Promise, um mehrfaches Laden zu verhindern
let spellsLoadingPromise = null;

// Funktion zum Laden der Spells vom Backend
async function loadAllSpells() {
    if (allSpellsData !== null) return allSpellsData;
    if (spellsLoadingPromise !== null) return spellsLoadingPromise;

    console.log("Fetching spell data from API...");

    // === KORREKTUR: Vollständige Backend-URL verwenden ===
    const backendUrl = 'http://localhost:8080/api/spells'; // Dein Backend-Server!
    // ===================================================

    spellsLoadingPromise = fetch(backendUrl) // Verwende die vollständige URL
        .then(response => {
            if (!response.ok) {
                // Versuche bei Fehlern, mehr Details zu loggen
                return response.text().then(text => {
                     console.error("Backend Error Response Text:", text);
                     throw new Error(`HTTP error fetching spells! status: ${response.status}`);
                });
            }
            // Versuche zuerst, als Text zu lesen, um zu sehen, was ankommt
            // return response.text().then(text => {
            //    console.log("Raw spell response text:", text.substring(0, 500) + "..."); // Logge Anfang des Texts
            //    return JSON.parse(text); // Dann parsen
            // });
            return response.json(); // Direktes Parsen
        })
        .then(data => {
            console.log("Spell data fetched successfully.");
            allSpellsData = data;
            spellsLoadingPromise = null;
            return allSpellsData;
        })
        .catch(error => {
            console.error('Failed to load spell data:', error);
            allSpellsData = null;
            spellsLoadingPromise = null;
            return null; // Gib null zurück bei Fehler
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