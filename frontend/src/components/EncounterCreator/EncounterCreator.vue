<script setup>
import { ref, computed, watch } from 'vue';
import MonsterFilter from './MonsterFilter.vue';
import MonsterList from './MonsterList.vue';
import EncounterDetails from './EncounterDetails.vue';
import AddedMonsterList from './AddedMonsterList.vue';
import EncounterSummary from './EncounterSummary.vue';

// --- Reaktive Daten (State) ---

// Master-Liste aller Monster vom Backend
const allMonsters = ref([]); // Format: [{ id, name, cr, size, type }, ...]
const isLoadingMonsters = ref(false);
const monsterLoadError = ref(null);

// Filterkriterien (werden von MonsterFilter aktualisiert)
const nameFilter = ref('');
const crFilterRange = ref([0, 30]); // Beispiel: Start mit vollem CR-Bereich

// Aktueller Encounter im Aufbau
const encounterId = ref(null); // Null für neuen Encounter, ID für bearbeiteten
const encounterName = ref('');
const encounterDescription = ref('');
const partySize = ref(4);
const partyLevel = ref(5);
const addedMonsters = ref([]); // Format: [{ monsterId, name, cr, count }, ...]

// Status für Speichern/Löschen/Laden
const isSaving = ref(false);
const saveError = ref(null);
const isDeleting = ref(false);
const deleteError = ref(null);
const isLoadingEncounter = ref(false);
const loadEncounterError = ref(null);


// --- Computed Properties ---

// Filtert die Master-Liste basierend auf den Kriterien
const filteredMonsters = computed(() => {
  return allMonsters.value.filter(monster => {
    const nameMatch = monster.name.toLowerCase().includes(nameFilter.value.toLowerCase());
    const crMatch = monster.cr >= crFilterRange.value[0] && monster.cr <= crFilterRange.value[1];
    return nameMatch && crMatch;
  }).sort((a, b) => { // Sortieren nach CR, dann Name
    if (a.cr !== b.cr) {
      return a.cr - b.cr;
    }
    return a.name.localeCompare(b.name);
  });
});

// Berechnet Schwierigkeit, XP etc. (Platzhalter - Logik muss implementiert werden!)
const calculatedStats = computed(() => {
  // TODO: Implementiere die D&D 5e Encounter-Schwierigkeitsberechnung
  // Benötigt: addedMonsters (CR, count), partySize, partyLevel
  // Referenz: Dungeon Master's Guide (DMG) Kapitel 3 oder Online-Rechner
  let totalXP = 0;
  let adjustedXP = 0; // XP angepasst für Monsteranzahl
  let monsterCount = 0;
  addedMonsters.value.forEach(m => {
      const xpForCR = getXPForCR(m.cr); // Hilfsfunktion nötig
      totalXP += xpForCR * m.count;
      monsterCount += m.count;
  });

  const multiplier = getXPMultiplier(monsterCount); // Hilfsfunktion nötig
  adjustedXP = totalXP * multiplier;

  const thresholds = getXPThresholds(partyLevel.value, partySize.value); // Hilfsfunktion nötig

  let difficulty = 'Trivial';
  if (adjustedXP >= thresholds.deadly) difficulty = 'Deadly';
  else if (adjustedXP >= thresholds.hard) difficulty = 'Hard';
  else if (adjustedXP >= thresholds.medium) difficulty = 'Medium';
  else if (adjustedXP >= thresholds.easy) difficulty = 'Easy';

  // Fortschritt zur nächsten Stufe (Beispiel)
  let progress = 0;
  let nextThreshold = thresholds.easy;
  if (difficulty === 'Easy') nextThreshold = thresholds.medium;
  else if (difficulty === 'Medium') nextThreshold = thresholds.hard;
  else if (difficulty === 'Hard') nextThreshold = thresholds.deadly;
  else if (difficulty === 'Deadly') nextThreshold = adjustedXP; // Max erreicht

  if (nextThreshold > 0) {
      let prevThreshold = 0;
      if (difficulty === 'Easy') prevThreshold = 0;
      else if (difficulty === 'Medium') prevThreshold = thresholds.easy;
      else if (difficulty === 'Hard') prevThreshold = thresholds.medium;
      else if (difficulty === 'Deadly') prevThreshold = thresholds.hard;

      if(nextThreshold - prevThreshold > 0) {
           progress = ((adjustedXP - prevThreshold) / (nextThreshold - prevThreshold)) * 100;
      } else {
           progress = (difficulty === 'Deadly') ? 100 : 0;
      }

  }


  return {
    difficulty: difficulty,
    totalXP: totalXP,
    adjustedXP: adjustedXP, // Angepasste XP für Schwierigkeit
    progress: Math.max(0, Math.min(progress, 100)), // Stelle sicher 0-100
    thresholds: thresholds // Zum Anzeigen (optional)
  };
});

// --- Hilfsfunktionen für Berechnung (Beispiele - müssen korrekt implementiert werden!) ---
function getXPForCR(cr) {
    // Mapping von CR zu XP (DMG S. 274-275) - Beispiel, vervollständigen!
    const xpMap = { 0: 10, 0.125: 25, 0.25: 50, 0.5: 100, 1: 200, 2: 450, 3: 700, 4: 1100, 5: 1800, /*...*/ 30: 155000 };
    return xpMap[cr] || 0;
}
function getXPMultiplier(monsterCount) {
    // Multiplikator basierend auf Anzahl (DMG S. 82) - Beispiel
    if (monsterCount <= 1) return 1;
    if (monsterCount === 2) return 1.5;
    if (monsterCount <= 6) return 2;
    if (monsterCount <= 10) return 2.5;
    if (monsterCount <= 14) return 3;
    return 4;
}
function getXPThresholds(level, count) {
    // Schwellenwerte pro Charakter (DMG S. 82) - Beispiel für Level 5
    const thresholdsPerLevel = { /* ... füllen für alle Level ... */ 5: { easy: 250, medium: 500, hard: 750, deadly: 1100 } };
    const base = thresholdsPerLevel[level] || { easy: 0, medium: 0, hard: 0, deadly: 0 };
    return {
        easy: base.easy * count,
        medium: base.medium * count,
        hard: base.hard * count,
        deadly: base.deadly * count,
    };
}

// --- Methoden zum Laden der Monsterliste ---
async function fetchAllMonsters() {
  isLoadingMonsters.value = true;
  monsterLoadError.value = null;
  try {
    // API CALL: Hole die zusammengefasste Monsterliste
    const response = await fetch('http://localhost:8080/api/monsters/summary'); // ANNAHME: Neuer Endpunkt
    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    allMonsters.value = await response.json();
  } catch (err) {
    console.error("Error fetching monster list:", err);
    monsterLoadError.value = "Failed to load monster list.";
  } finally {
    isLoadingMonsters.value = false;
  }
}

// --- Methoden zum Interagieren mit dem Encounter ---

function handleFilterChanged(filters) {
  nameFilter.value = filters.name;
  crFilterRange.value = filters.crRange;
}

function handleAddMonster(monsterData) {
  // Finde, ob Monster schon in der Liste ist
  const existingMonster = addedMonsters.value.find(m => m.monsterId === monsterData.id);
  if (existingMonster) {
    existingMonster.count++;
  } else {
    // Füge neu hinzu (nur nötige Daten speichern)
    addedMonsters.value.push({
      monsterId: monsterData.id,
      name: monsterData.name,
      cr: monsterData.cr,
      count: 1
    });
  }
}

function handleUpdateMonsterCount({ monsterId, newCount }) {
   const monster = addedMonsters.value.find(m => m.monsterId === monsterId);
   if (monster) {
       if (newCount > 0) {
           monster.count = newCount;
       } else {
           // Bei 0 entfernen
           handleRemoveMonster(monsterId);
       }
   }
}

function handleRemoveMonster(monsterId) {
  addedMonsters.value = addedMonsters.value.filter(m => m.monsterId !== monsterId);
}

// Funktion zum Laden eines vorhandenen Encounters (vereinfacht)
async function handleLoadEncounter(idToLoad) {
  if (!idToLoad) return;
  isLoadingEncounter.value = true;
  loadEncounterError.value = null;
  clearEncounter(); // Erstmal alles leeren

  try {
    const response = await fetch(`http://localhost:8080/api/encounters/${idToLoad}`);
    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    const data = await response.json();

    // Encounter-Daten übernehmen
    encounterId.value = data.id;
    encounterName.value = data.name || '';
    encounterDescription.value = data.description || '';
    if (data.party) {
      partySize.value = data.party.playerCount || 4;
      partyLevel.value = data.party.averageLevel || 5;
    }
    // Monsterliste übernehmen (wir brauchen nur ID und Count, der Rest wird über allMonsters geholt)
    addedMonsters.value = data.monsters.map(m => {
        const baseMonster = allMonsters.value.find(bm => bm.id === m.monsterId);
        return {
           monsterId: m.monsterId,
           name: baseMonster?.name || m.monsterId, // Fallback auf ID
           cr: baseMonster?.cr || 0,             // Fallback auf 0
           count: m.count
        }
    });

  } catch(err) {
    console.error("Error loading encounter for editing:", err);
    loadEncounterError.value = "Failed to load encounter.";
    clearEncounter(); // Bei Fehler wieder leeren
  } finally {
    isLoadingEncounter.value = false;
  }
}

// Hilfsfunktion zum Zurücksetzen des Formulars
function clearEncounter() {
  encounterId.value = null;
  encounterName.value = '';
  encounterDescription.value = '';
  partySize.value = 4;
  partyLevel.value = 5;
  addedMonsters.value = [];
  saveError.value = null;
  deleteError.value = null;
  loadEncounterError.value = null;
}

// --- Methoden zum Speichern und Löschen ---
async function handleSaveEncounter() {
  isSaving.value = true;
  saveError.value = null;

  // ID generieren, falls es ein neuer Encounter ist
  const idToSave = encounterId.value || encounterName.value.toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/^_|_$/g, '') || `encounter_${Date.now()}`;

  // Datenpaket für das Backend vorbereiten
  const encounterData = {
      id: idToSave,
      name: encounterName.value,
      description: encounterDescription.value,
      party: {
          playerCount: partySize.value,
          averageLevel: partyLevel.value
      },
      // Wichtig: Nur MonsterId und count senden! Keine Statblocks!
      monsters: addedMonsters.value.map(m => ({ monsterId: m.monsterId, count: m.count })),
      calculatedDifficulty: calculatedStats.value.difficulty // Speichere die berechnete Schwierigkeit mit
  };

  try {
      // API CALL: Speichern (UPSERT)
      const response = await fetch('http://localhost:8080/api/encounters', { // Einfacher POST-Endpunkt für Upsert
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(encounterData)
      });

      if (!response.ok) {
          const errorData = await response.json().catch(() => ({ message: 'Unknown error' }));
          throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || 'Save failed'}`);
      }

      // Erfolgreich gespeichert -> ID aktualisieren, falls es ein neuer war
      encounterId.value = idToSave;
      // Optional: Erfolgsmeldung anzeigen
      console.log("Encounter saved successfully!");


  } catch (err) {
      console.error("Error saving encounter:", err);
      saveError.value = err.message || "Failed to save encounter.";
  } finally {
      isSaving.value = false;
  }
}

async function handleDeleteEncounter() {
   if (!encounterId.value) {
       deleteError.value = "Cannot delete an unsaved encounter.";
       return;
   }
   if (!confirm(`Are you sure you want to delete encounter "${encounterName.value}"?`)) {
       return;
   }

   isDeleting.value = true;
   deleteError.value = null;

   try {
       // API CALL: Löschen
       const response = await fetch(`http://localhost:8080/api/encounters/${encounterId.value}`, {
           method: 'DELETE'
       });
        if (!response.ok) {
            const errorData = await response.json().catch(() => ({ message: 'Unknown error' }));
            throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || 'Delete failed'}`);
        }
        // Erfolgreich gelöscht -> Formular leeren
        clearEncounter();
        console.log("Encounter deleted successfully!");
        // Optional: Encounter-Liste neu laden im EncounterDetails (falls Dropdown verwendet wird)

   } catch(err) {
        console.error("Error deleting encounter:", err);
        deleteError.value = err.message || "Failed to delete encounter.";
   } finally {
       isDeleting.value = false;
   }
}


// --- Lifecycle Hooks ---
import { onMounted } from 'vue';
onMounted(() => {
  fetchAllMonsters(); // Lade die Monsterliste beim Start
  // Optional: Lade auch die Liste der existierenden Encounters für den Selector
});

</script>

<template>
  <v-container fluid>
    <v-row>
      <!-- Spalte 1: Monster Filter & Liste -->
      <v-col cols="12" md="4">
        <MonsterFilter @filter-changed="handleFilterChanged" />
        <v-divider class="my-4"></v-divider>
        <div v-if="isLoadingMonsters">Loading monsters...</div>
        <v-alert v-if="monsterLoadError" type="error">{{ monsterLoadError }}</v-alert>
        <MonsterList v-else :monsters="filteredMonsters" @add-monster="handleAddMonster" />
      </v-col>

      <!-- Spalte 2: Encounter Details -->
      <v-col cols="12" md="4">
         <EncounterDetails
             :name="encounterName"
             :description="encounterDescription"
             :party-size="partySize"
             :party-level="partyLevel"
             :loading="isLoadingEncounter"
             :error="loadEncounterError"
             @update:name="encounterName = $event"
             @update:description="encounterDescription = $event"
             @update:party-size="partySize = $event"
             @update:party-level="partyLevel = $event"
             @load-encounter="handleLoadEncounter"
             @clear-encounter="clearEncounter"
         />
         <!-- Zeige Speicher/Löschfehler an -->
         <v-alert v-if="saveError" type="error" density="compact" class="mt-2">{{ saveError }}</v-alert>
         <v-alert v-if="deleteError" type="error" density="compact" class="mt-2">{{ deleteError }}</v-alert>
      </v-col>

      <!-- Spalte 3: Hinzugefügte Monster & Übersicht -->
      <v-col cols="12" md="4" class="d-flex flex-column">
          <div style="flex-grow: 1; overflow-y: auto;"> <!-- Scrollbarer Bereich für Monster -->
            <AddedMonsterList
              :added-monsters="addedMonsters"
              @update-count="handleUpdateMonsterCount"
              @remove-monster="handleRemoveMonster"
             />
          </div>
          <div style="flex-shrink: 0;"> <!-- Fester Bereich für Summary -->
             <EncounterSummary
               :difficulty="calculatedStats.difficulty"
               :total-xp="calculatedStats.totalXP"
               :adjusted-xp="calculatedStats.adjustedXP"
               :progress="calculatedStats.progress"
               :is-saving="isSaving"
               :is-deleting="isDeleting"
               :can-delete="!!encounterId" 
               @save="handleSaveEncounter"
               @delete="handleDeleteEncounter"
             />
          </div>
      </v-col>
    </v-row>
  </v-container>
</template>

<style scoped>
/* Stile hier, falls nötig */
/* Stelle sicher, dass die Spalte 3 Höhe hat, damit Scrollen funktioniert */
.d-flex.flex-column {
    height: calc(100vh - 120px); /* Beispiel: Höhe des Viewports minus App-Bar Höhe, anpassen! */
    max-height: calc(100vh - 120px);
}
</style>