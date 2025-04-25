// frontend/src/utils/dndDataService.js
import { ref } from 'vue';

// Cache für geladene Daten
const dataCache = ref({});
const loadingPromises = {};

// Basis-URL für die API
const API_BASE_URL = 'http://localhost:8080/api'; // Passe Port an, falls nötig

/**
 * Lädt eine spezifische DnDData JSON-Datei vom Backend.
 * Verwendet einen Cache, um wiederholte Anfragen zu vermeiden.
 * @param {string} filename Der Dateiname (z.B. 'skills.json', 'crData.json')
 * @returns {Promise<object|array|null>} Ein Promise, das mit den geparsten JSON-Daten oder null bei Fehler resolved.
 */
export async function loadDnDData(filename) {
    if (!filename) {
        console.error("loadDnDData: Filename is required.");
        return null;
    }

    // Prüfe Cache
    if (dataCache.value[filename]) {
        // console.log(`DnDDataService: Returning cached data for ${filename}`);
        return dataCache.value[filename];
    }

    // Prüfe laufende Promises
    if (loadingPromises[filename]) {
        // console.log(`DnDDataService: Waiting for existing promise for ${filename}`);
        return loadingPromises[filename];
    }

    console.log(`DnDDataService: Fetching ${filename} from API...`);
    loadingPromises[filename] = fetch(`${API_BASE_URL}/dnddata/${filename}`)
        .then(response => {
            if (!response.ok) {
                 // Versuche, die Fehlermeldung vom Server zu lesen
                 return response.text().then(text => {
                     console.error(`Failed to fetch ${filename}. Status: ${response.status}. Response: ${text}`);
                     throw new Error(`HTTP error fetching ${filename}! status: ${response.status}`);
                 });
            }
            return response.json();
        })
        .then(data => {
            console.log(`DnDDataService: Successfully fetched ${filename}.`);
            dataCache.value[filename] = data; // Speichere im Cache
            delete loadingPromises[filename]; // Entferne Promise nach Erfolg
            return data;
        })
        .catch(error => {
            console.error(`DnDDataService: Failed to load ${filename}:`, error);
            delete loadingPromises[filename]; // Entferne Promise nach Fehler
            return null; // Gib null zurück bei Fehler
        });

    return loadingPromises[filename];
}

// Optional: Funktion zum Vorladen wichtiger Daten beim App-Start
export function preloadCommonData() {
    console.log("DnDDataService: Preloading common data...");
    loadDnDData('skills.json');
    loadDnDData('crData.json');
    loadDnDData('sizes.json');
    loadDnDData('stats.json');
    loadDnDData('monsterTypes.json');
    loadDnDData('alignments.json');
    loadDnDData('acTypes.json');
    loadDnDData('damageTypes.json');
    loadDnDData('conditionResImuVul.json');
    loadDnDData('languages.json');
    // ... füge weitere häufig benötigte Dateien hinzu ...
}

// Rufe preload beim Importieren auf (optional)
// preloadCommonData();