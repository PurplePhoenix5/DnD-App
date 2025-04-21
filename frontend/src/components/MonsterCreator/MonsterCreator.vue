<!-- frontend/src/components/MonsterCreator/MonsterCreator.vue -->
<script setup>
import { ref, reactive, watch } from 'vue'; // watch hinzugefügt
import { Splitpanes, Pane } from 'splitpanes';
import 'splitpanes/dist/splitpanes.css'; // Importiere das CSS für Splitpanes
import MonsterLoader from './MonsterLoader.vue'; // NEU
import MonsterConfigurator from './MonsterConfigurator.vue';
import StatBlockRenderer from '../StatBlockRenderer.vue';

// --- Reaktiver Zustand für das Monster ---
const monsterBeingCreated = reactive({
  // ... (Initialisiere mit Standardwerten wie zuvor) ...
  name: 'New Creature', saveVersion: 10, size: 'Medium', type: 'Humanoid', alignment: 'unaligned',
  AC: 10, ACType: 'natural armor', CR: 0, proficiency: 2,
  HP: { HD: 1, type: 8, modifier: 0 },
  stats: { STR: 10, DEX: 10, CON: 10, INT: 10, WIS: 10, CHA: 10 },
  speeds: [{ id: 'speed_walk', type: 'walk', speed: 30, note: '' }],
  saves: { STR: { proficient: false, override: false, overrideValue: 0 }, DEX: { proficient: false, override: false, overrideValue: 0 }, CON: { proficient: false, override: false, overrideValue: 0 }, INT: { proficient: false, override: false, overrideValue: 0 }, WIS: { proficient: false, override: false, overrideValue: 0 }, CHA: { proficient: false, override: false, overrideValue: 0 } },
  skills: [], resistances: [], immunities: [], vulnerabilities: [], conditions: [],
  senses: { blindsight: 0, darkvision: 0, tremorsense: 0, truesight: 0 },
  passivePerception: { override: false, overrideValue: 10 }, languages: '',
  traits: [], actions: [], attacks: [],
  spellcasting: { stat: 'INT', save: { override: false, overrideValue: 0 }, modifier: { override: false, overrideValue: 0 }, attack: { override: false, overrideValue: 0 }, level: 1, slots: [0,0,0,0,0,0,0,0,0], atWill: [], standard: [], notes: '', atWillNotes: '' },
  multiattacks: [], multiattackOptions: {}, legendaryActions: { count: 0, actions: [] },
  mythicActions: { triggerName: '', triggerRecharge: '', triggerDescription: '', preamble: '', actions: []}, reactions: [], lairActions: [], regionalEffects: [],
  // ... (ggf. weitere Felder)
});

// --- NEU: Status für das Laden eines spezifischen Monsters ---
const isLoadingMonsterDetails = ref(false);
const loadMonsterDetailsError = ref(null);
// ===========================================================

// --- Bestehende Speicher-Status (Platzhalter) ---
const isSaving = ref(false);
const saveError = ref(null);
// =============================================

// === NEU: Refs für Style und Columns ===
const displayStyle = ref('2024');   // Default Stil
const displayColumns = ref(1);     // Default Spalten


// --- Methoden ---

// === NEU: Methode zum Laden der Monsterdetails ===
async function handleLoadMonster(monsterId) {
    if (!monsterId) return;
    console.log('MonsterCreator: Attempting to load monster with ID:', monsterId);
    isLoadingMonsterDetails.value = true;
    loadMonsterDetailsError.value = null;

    try {
        // API CALL: Hole die *vollen* Monsterdaten
        const response = await fetch(`http://localhost:8080/api/monsters/${monsterId}`);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        console.log('MonsterCreator: Received monster details:', data);

        // --- Aktualisiere das reactive monsterBeingCreated Objekt ---
        // Wichtig: Direkte Zuweisung monsterBeingCreated = data würde die Reaktivität brechen.
        // Stattdessen müssen wir die Eigenschaften aktualisieren.

        // 1. Leere das Objekt (optional, aber sauberer für Arrays etc.)
        //    Behalte aber die Grundstruktur bei, falls die API nicht *alle* Felder liefert.
        Object.keys(monsterBeingCreated).forEach(key => {
            // Lösche Arrays oder setze auf leere Objekte/Strings/Null zurück,
            // behalte aber primitive Defaults, falls nicht in API vorhanden.
             if (Array.isArray(monsterBeingCreated[key])) {
                 monsterBeingCreated[key] = [];
             } else if (typeof monsterBeingCreated[key] === 'object' && monsterBeingCreated[key] !== null) {
                 // Bei verschachtelten Objekten vorsichtig sein oder leer lassen
                 // monsterBeingCreated[key] = {}; // Vorsicht hiermit!
             } else if (typeof monsterBeingCreated[key] === 'string') {
                 monsterBeingCreated[key] = '';
             } else if (typeof monsterBeingCreated[key] === 'number') {
                 // Behalte ggf. Defaults wie 0 oder 10
                 // monsterBeingCreated[key] = 0;
             } else {
                 // monsterBeingCreated[key] = null;
             }
        });

        // 2. Weise die geladenen Daten zu (überschreibt/fügt hinzu)
        // Object.assign ist flach, besser ist es, Feld für Feld zu gehen oder deep merge
        // Einfacher Ansatz: Kopiere alle Top-Level Felder
        for (const key in data) {
            if (Object.hasOwnProperty.call(data, key)) {
                 // Prüfe ob das Feld im reactive Objekt existiert, um Typfehler zu vermeiden
                 // oder erweitere das reactive Objekt dynamisch (kann riskant sein)
                 // if (key in monsterBeingCreated) {
                     monsterBeingCreated[key] = data[key];
                 // }
            }
        }
        // Stelle sicher, dass notwendige verschachtelte Objekte existieren, falls API sie weglässt
         if (!monsterBeingCreated.HP) monsterBeingCreated.HP = { HD: 1, type: 8, modifier: 0 };
         if (!monsterBeingCreated.stats) monsterBeingCreated.stats = { STR: 10, DEX: 10, CON: 10, INT: 10, WIS: 10, CHA: 10 };
         // ... usw. für andere wichtige verschachtelte Strukturen wie saves, senses, spellcasting ...


        console.log('MonsterCreator: Updated monsterBeingCreated:', JSON.parse(JSON.stringify(monsterBeingCreated)));


    } catch (err) {
        console.error("Error loading monster details:", err);
        loadMonsterDetailsError.value = `Failed to load details for ${monsterId}.`;
        // Optional: Setze monsterBeingCreated zurück auf Standardwerte
    } finally {
        isLoadingMonsterDetails.value = false;
    }
}
// ========================================

async function saveMonster() {
  console.log("Saving Monster:", JSON.parse(JSON.stringify(monsterBeingCreated)));
  isSaving.value = true;
  saveError.value = null;

  // --- Bestimme die ID für die PUT-Anfrage ---
  // Annahme: Die ID ist jetzt Teil von monsterBeingCreated, wenn ein Monster geladen wurde
  // oder muss aus dem Namen generiert/validiert werden.
  // Hier nehmen wir an, dass die ID aus dem Namen abgeleitet wird, wenn nicht vorhanden.
  let monsterIdToSave = monsterBeingCreated.id; // ID könnte nach Laden vorhanden sein
  if (!monsterIdToSave && monsterBeingCreated.name) {
     monsterIdToSave = monsterBeingCreated.name.toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/^_|_$/g, '');
     // Füge die generierte ID zum Objekt hinzu, falls noch nicht vorhanden
     monsterBeingCreated.id = monsterIdToSave;
  }

  if (!monsterIdToSave) {
      saveError.value = "Cannot save monster without a name or ID.";
      isSaving.value = false;
      return;
  }

  try {
      // API CALL: Verwende PUT für Upsert
      const response = await fetch(`http://localhost:8080/api/monsters/${monsterIdToSave}`, {
           method: 'PUT',
           headers: { 'Content-Type': 'application/json' },
           body: JSON.stringify(monsterBeingCreated) // Sende das komplette Objekt
      });

       if (!response.ok) {
           const errorData = await response.json().catch(() => ({ message: 'Unknown error during save' }));
           throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || 'Save failed'}`);
       }

       const savedData = await response.json(); // Die vom Server bestätigten Daten
       // Aktualisiere unser lokales Objekt mit den Daten vom Server (könnte ID enthalten)
       Object.assign(monsterBeingCreated, savedData);

       console.log("Monster saved/updated successfully!", savedData);
       // Optional: Erfolgsmeldung anzeigen

  } catch(err) {
      console.error("Error saving monster:", err);
      saveError.value = err.message || "Failed to save monster.";
  } finally {
       isSaving.value = false;
  }

}

</script>

<template>
  <v-container fluid>
    <!-- === NEUE REIHE für Monster Loader === -->
    <v-row>
        <v-col cols="12">
            <MonsterLoader
                @load-monster="handleLoadMonster"
                v-model:style="displayStyle"
                v-model:columns="displayColumns"
             />
             <!-- Optional: Zeige Fehler beim Laden der Details -->
             <v-alert v-if="loadMonsterDetailsError" type="error" density="compact" class="mt-2">
                {{ loadMonsterDetailsError }}
             </v-alert>
        </v-col>
    </v-row>
    <!-- ===================================== -->

    <!-- Bestehende Reihe für Konfiguration und Vorschau -->
    <splitpanes class="default-theme" style="height: calc(100vh - 200px);">
        <!-- Pane 1: Konfiguration -->
        <pane size="50"> <!-- Startet mit 50% Breite -->
            <!-- Hier kommt der Inhalt der alten Spalte 1 rein -->
            <div class="pa-2 bg-surface" style="height: 100%; overflow-y: auto;">
                <MonsterConfigurator
                
                />
                <!-- Speicher-Button -->
                <v-btn
                    color="primary"
                    @click="saveMonster"
                    :loading="isSaving"
                    :disabled="isSaving"
                    class="mt-4"
                >
                    Save Monster
                </v-btn>
                <!-- Zeige Speicherfehler -->
                <v-alert v-if="saveError" type="error" density="compact" class="mt-2">
                    {{ saveError }}
                </v-alert>
            </div>
        </pane>
        <!-- Pane 2: Statblock Vorschau -->
        <pane size="50"> <!-- Startet mit 50% Breite -->
                <div class="pa-2 bg-surface" style="height: 100%; overflow-y: auto;"> <!-- Padding & Scroll -->
                    <v-card variant="outlined">
                        <!-- Zeige Ladeindikator für Monsterdetails -->
                        <div v-if="isLoadingMonsterDetails" class="text-center pa-5">
                            <v-progress-circular indeterminate color="primary"></v-progress-circular>
                            <p>Loading monster details...</p>
                        </div>
                        <!-- Zeige Renderer, wenn nicht geladen wird -->
                        <v-card-text v-else>
                            <StatBlockRenderer
                                :monster-data="monsterBeingCreated"
                                :display-style="displayStyle" 
                                :columns="displayColumns"
                            />
                        </v-card-text>
                 </v-card>
             </div>
        </pane>
    </splitpanes>
  </v-container>
</template>

<style scoped>
/* Splitpanes benötigt oft eine explizite Höhe */
/* Passe die Höhe an, ziehe z.B. die Höhe der App-Bar und des Loaders ab */
/* Der Wert '200px' ist eine Schätzung, passe ihn ggf. an! */
.splitpanes.default-theme {
  /* height: calc(100vh - 200px);  */ /* Höhe im style-Attribut gesetzt */
}

/* Styling für den Splitter (optional, default-theme bringt schon was mit) */
.splitpanes.default-theme .splitpanes__splitter {
  background-color: #ccc;
  position: relative;
}
.splitpanes.default-theme .splitpanes__splitter:before {
  content: '';
  position: absolute;
  left: 0;
  top: 0;
  transition: opacity 0.4s;
  background-color: rgba(0, 0, 0, 0.2);
  opacity: 0;
  z-index: 1;
}
.splitpanes.default-theme .splitpanes__splitter:hover:before {
  opacity: 1;
}
.splitpanes--vertical > .splitpanes__splitter:before {
  left: -1px;
  right: -1px;
  height: 100%;
}
.splitpanes--horizontal > .splitpanes__splitter:before {
  top: -1px;
  bottom: -1px;
  width: 100%;
}

/* Stelle sicher, dass der Inhalt scrollbar ist, falls er überläuft */
.pane-content {
    height: 100%;
    overflow-y: auto;
    padding: 8px; /* Füge Padding zum Inhalt hinzu */
}
.statblock-container {
    height: calc(100% - 40px); /* Höhe abzüglich Card-Title */
    overflow-y: auto;
}
</style>