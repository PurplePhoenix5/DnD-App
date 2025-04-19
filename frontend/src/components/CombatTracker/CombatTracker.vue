<script setup>
import { ref, computed, watch } from 'vue';
// Importiere hier die anderen Komponenten, wenn sie erstellt sind
import EncounterSelector from './EncounterSelector.vue';
import InitiativeList from './InitiativeList.vue';
import InteractionPanel from './InteractionPanel.vue';
import StatBlockDisplay from './StatBlockDisplay.vue';


// --- Refaktive Daten (State) ---
const availableEncounters = ref([]); // [{ id: 'enc1', name: 'Encounter 1' }, ...]
const selectedEncounterId = ref(null);
const loadedEncounterData = ref(null); // Das volle Objekt vom Backend für den ausgewählten Encounter
const combatants = ref([]); // Aktive Kreaturen im Kampf: [{ id: 'goblin_1', baseMonsterId: 'goblin', name: 'Goblin 1', initiative: 15, currentHp: 7, maxHp: 7, ac: 15, cr: 0.25, statusEffects: [] }, ...]
const selectedCombatantId = ref(null);
const loadedDifficulty = ref('N/A'); // Wird aus der Encounter-Datei gelesen
// const damageHealAmount = ref(0); // Nicht mehr direkt hier benötigt, da InteractionPanel es selbst verwaltet
const isLoading = ref(false);
const error = ref(null);

// --- Computed Properties (Abgeleitete Daten) ---
const selectedCombatant = computed(() => {
  return combatants.value.find(c => c.id === selectedCombatantId.value) || null;
});

const selectedConditions = computed(() => {
   return selectedCombatant.value?.statusEffects || []; // Gibt leeres Array zurück, wenn niemand ausgewählt ist
});

const uniqueMonsterStatBlocks = computed(() => {
  // Diese Funktion muss jetzt anders arbeiten, da `loadedEncounterData` keinen `statBlock` mehr direkt enthält.
  // Wir brauchen einen Weg, die vollen Statblocks bei Bedarf nachzuladen oder aus einem Cache zu holen.
  // Vorerst geben wir ein leeres Array zurück, da die Anzeige der Statblocks (Spalte 3)
  // mit der aktuellen Datenstruktur nicht mehr direkt funktioniert.
  // TODO: Statblock-Anzeige überarbeiten (z.B. Nachladen bei Klick oder separaten API-Call)

  // ---- Alte Logik (funktioniert nicht mehr) ----
  /*
  if (!loadedEncounterData.value || !loadedEncounterData.value.monsters) return [];
  const uniqueBlocks = [];
  const seenMonsterIds = new Set();
  loadedEncounterData.value.monsters.forEach(monsterDef => {
    // PROBLEM: monsterDef enthält keinen statBlock mehr direkt!
    if (monsterDef.statBlock && !seenMonsterIds.has(monsterDef.monsterId)) {
       uniqueBlocks.push(monsterDef.statBlock);
       seenMonsterIds.add(monsterDef.monsterId);
     }
   });
   uniqueBlocks.sort((a, b) => (b.CR ?? 0) - (a.CR ?? 0));
   return uniqueBlocks;
   */
   // ---- Temporäre Lösung: Leeres Array ----
   console.warn("uniqueMonsterStatBlocks computed property needs rework. StatBlocks are not directly in loadedEncounterData anymore.");
   return []; // Gib vorerst leer zurück
});

// --- Methoden ---
async function fetchAndLoadEncounter(encounterId) {
  if (!encounterId) return;
  isLoading.value = true;
  error.value = null;
  selectedEncounterId.value = encounterId;
  selectedCombatantId.value = null; // Auswahl zurücksetzen
  combatants.value = []; // Alte Liste leeren
  loadedEncounterData.value = null;
  loadedDifficulty.value = 'N/A';

  try {
    // --- API CALL ---
    const response = await fetch(`http://localhost:8080/api/encounters/${encounterId}`);
    console.log('Fetch response status:', response.status); // Log Status
    if (!response.ok) {
       const errorText = await response.text(); // Versuche Text statt JSON bei Fehler
       console.error('Fetch error response text:', errorText);
       throw new Error(`HTTP error! status: ${response.status}`);
    }
    const data = await response.json();
    console.log('Received encounter data:', JSON.parse(JSON.stringify(data))); // Log die empfangenen Daten

    loadedEncounterData.value = data; // Speichere die Rohdaten (jetzt mit angereicherten Monstern)
    loadedDifficulty.value = data.calculatedDifficulty || 'Unknown';

    // Erzeuge die Combatant-Liste aus den Encounter-Daten
    const newCombatants = [];
    let combatantCounter = 1; // Zur Generierung eindeutiger IDs pro Instanz

     // Sicherer Zugriff auf das Monster-Array
    if (data.monsters && Array.isArray(data.monsters)) {
      data.monsters.forEach(monsterInEncounter => { // Variable umbenannt zur Klarheit
        console.log('Processing monster from encounter:', monsterInEncounter); // Log zum Debuggen

        // Prüfe auf notwendige Felder direkt im monsterInEncounter Objekt
        if (!monsterInEncounter.monsterId || !monsterInEncounter.count || !monsterInEncounter.name) {
           console.warn("Skipping invalid monster entry in encounter:", monsterInEncounter);
           return; // Überspringe diesen Eintrag
        }

        for (let i = 0; i < monsterInEncounter.count; i++) {
          const uniqueId = `${monsterInEncounter.monsterId}_${combatantCounter++}`;

          // === KORREKTUR: Werte direkt aus monsterInEncounter lesen ===
          newCombatants.push({
            id: uniqueId, // Eindeutige ID für *diese Instanz* im Kampf
            baseMonsterId: monsterInEncounter.monsterId, // ID des Monstertyps
            name: `${monsterInEncounter.name} ${i + 1}`, // Angepasster Name für Instanz
            // Initiative: Verwende den gespeicherten Bonus und würfle d20
            initiative: Math.floor(Math.random() * 20) + 1 + (monsterInEncounter.initiativeBonus ?? 0),
            // Verwende die gespeicherte averageHp
            currentHp: monsterInEncounter.averageHp ?? 10, // Fallback, falls fehlt
            maxHp: monsterInEncounter.averageHp ?? 10,     // Fallback, falls fehlt
            // Verwende die gespeicherte AC
            ac: monsterInEncounter.AC ?? 10,              // Fallback, falls fehlt
            // Verwende den gespeicherten CR
            cr: monsterInEncounter.CR ?? 0,                // Fallback, falls fehlt
            statusEffects: [], // Startet ohne Effekte
          });
          // ===========================================================
        }
      });
    } else {
       console.warn(`Kein gültiges 'monsters'-Array im Encounter ${encounterId} gefunden.`);
    }


    combatants.value = newCombatants;
    sortCombatants(); // Initial sortieren

  } catch (err) {
    // Logge den spezifischen Fehler, der aufgetreten ist
    console.error('Error in fetchAndLoadEncounter:', err);
    error.value = 'Failed to load or process encounter data.'; // Spezifischere UI-Meldung evtl.
    loadedEncounterData.value = null;
    combatants.value = [];
    loadedDifficulty.value = 'Error';
  } finally {
    isLoading.value = false;
  }
}

function handleEncounterSelected(encounterId) {
  fetchAndLoadEncounter(encounterId);
}

function handleSelectCombatant(combatantId) {
  selectedCombatantId.value = combatantId;
}

function handleUpdateInitiative({ combatantId, newInitiative }) {
  const combatant = combatants.value.find(c => c.id === combatantId);
  if (combatant) {
    // Stelle sicher, dass newInitiative eine Zahl ist und im gültigen Bereich liegt
    const parsedInitiative = parseInt(newInitiative, 10);
    if (!isNaN(parsedInitiative)) {
        // Annahme: MIN/MAX sind hier nicht global bekannt, einfache Prüfung
        combatant.initiative = Math.max(0, parsedInitiative);
        sortCombatants();
    }
  }
}

function handleApplyDamage(amount) {
  const combatant = selectedCombatant.value;
  if (combatant && amount > 0) {
    combatant.currentHp = Math.max(0, combatant.currentHp - amount);
  }
}

function handleApplyHealing(amount) {
  const combatant = selectedCombatant.value;
  if (combatant && amount > 0) {
    combatant.currentHp = Math.min(combatant.maxHp, combatant.currentHp + amount);
  }
}

function handleAddCondition(conditionId) {
  const combatant = selectedCombatant.value;
  if (combatant && !combatant.statusEffects.includes(conditionId)) {
      combatant.statusEffects.push(conditionId);
  }
}

function handleRemoveCondition(conditionId) {
  const combatant = selectedCombatant.value;
  if (combatant) {
    const index = combatant.statusEffects.indexOf(conditionId);
    if (index > -1) {
      combatant.statusEffects.splice(index, 1);
    }
  }
}

function sortCombatants() {
  combatants.value.sort((a, b) => b.initiative - a.initiative);
}

// --- Watcher (Optional) ---
watch(combatants, (newVal, oldVal) => {
  // Hier könnte Logik stehen, z.B. um automatisch den nächsten in der Initiative zu wählen,
  // wenn der aktuelle auf 0 HP fällt o.ä.
}, { deep: true });

</script>


<template>
  <v-container fluid>
    <v-row>
      <!-- Spalte 1: Encounter Auswahl & Initiative Liste -->
      <v-col cols="12" md="5">
        <EncounterSelector @encounter-selected="handleEncounterSelected" />
        <v-divider class="my-4"></v-divider>

        <div v-if="isLoading" class="text-center pa-4">
            Loading Encounter... <v-progress-circular indeterminate></v-progress-circular>
        </div>
        <v-alert v-if="error" type="error" density="compact">{{ error }}</v-alert>

        <InitiativeList
          v-if="!isLoading && combatants.length > 0"
          :combatants="combatants"
          :selected-combatant-id="selectedCombatantId"
          @select-combatant="handleSelectCombatant"
          @update-initiative="handleUpdateInitiative"
        />
        <div v-else-if="!isLoading && !error && selectedEncounterId" class="text-disabled pa-4 text-center">
          No combatants loaded for this encounter.
        </div>
         <div v-else-if="!isLoading && !error && !selectedEncounterId" class="text-disabled pa-4 text-center">
          Select an encounter to begin.
        </div>
      </v-col>

      <!-- Spalte 2: Interaktion -->
      <v-col cols="12" md="3">
        <InteractionPanel
          :encounter-difficulty-prop="loadedDifficulty"
          :selected-combatant-id="selectedCombatantId"
          :selected-combatant-conditions="selectedConditions"
          @apply-damage="handleApplyDamage"
          @apply-healing="handleApplyHealing"
          @add-condition="handleAddCondition"
          @remove-condition="handleRemoveCondition"
        />
      </v-col>

      <!-- Spalte 3: Stat Blocks -->
      <v-col cols="12" md="4">
         <!-- TODO: StatBlockDisplay muss überarbeitet werden, da uniqueMonsterStatBlocks jetzt leer ist -->
         <!-- Momentan wird hier nichts angezeigt, außer der Fallback-Text -->
         <StatBlockDisplay
           v-if="uniqueMonsterStatBlocks.length > 0"
           :unique-monster-stat-blocks="uniqueMonsterStatBlocks"
         />
         <div v-else-if="selectedEncounterId && !isLoading" class="text-disabled pa-4 text-center">
            <!-- Stat block display needs rework. -->
            (Stat Blocks temporarily unavailable)
         </div>
      </v-col>
    </v-row>
  </v-container>
</template>

<style scoped>
/* Füge hier bei Bedarf spezifische Stile hinzu */
</style>