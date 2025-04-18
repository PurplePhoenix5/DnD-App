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
const loadedDifficulty = ref('N/A');
const damageHealAmount = ref(0); // Gebunden an das Inputfeld in InteractionPanel
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
  if (!loadedEncounterData.value || !loadedEncounterData.value.monsters) return [];
  
  const uniqueBlocks = [];
  const seenMonsterIds = new Set();

  loadedEncounterData.value.monsters.forEach(monsterDef => {
    if (!seenMonsterIds.has(monsterDef.id)) {
      uniqueBlocks.push(monsterDef.statBlock);
      seenMonsterIds.add(monsterDef.id);
    }
  });
  
  // Optional: Nach CR sortieren
  uniqueBlocks.sort((a, b) => b.CR - a.CR); 
  return uniqueBlocks;
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
    const response = await fetch(`http://localhost:8080/api/encounters/${encounterId}`); // Passe Port/URL an
    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    const data = await response.json();
    loadedEncounterData.value = data; // Speichere die Rohdaten
    loadedDifficulty.value = data.calculatedDifficulty || 'Unknown';

    // Erzeuge die Combatant-Liste aus den Encounter-Daten
    const newCombatants = [];
    let combatantCounter = 1;
    data.monsters.forEach(monsterDef => {
      for (let i = 0; i < monsterDef.count; i++) {
        const uniqueId = `${monsterDef.id}_${combatantCounter++}`;
        newCombatants.push({
          id: uniqueId,
          baseMonsterId: monsterDef.id,
          name: `${monsterDef.statBlock.name} ${i + 1}`, // Eindeutiger Name
          initiative: Math.floor(Math.random() * 20) + 1 + (monsterDef.statBlock.stats.DEX ? Math.floor((monsterDef.statBlock.stats.DEX - 10) / 2) : 0), // Beispiel: Random Init + Dex Mod
          currentHp: monsterDef.statBlock.HP.HD * monsterDef.statBlock.HP.type / 2 + monsterDef.statBlock.HP.modifier, // Vereinfacht: Durchschnittliche HP
          maxHp: monsterDef.statBlock.HP.HD * monsterDef.statBlock.HP.type / 2 + monsterDef.statBlock.HP.modifier, // Vereinfacht: Durchschnittliche HP
          ac: monsterDef.statBlock.AC,
          cr: monsterDef.statBlock.CR, // Stelle sicher, dass CR im statBlock ist
          statusEffects: [], // Startet ohne Effekte
        });
      }
    });
    combatants.value = newCombatants;
    sortCombatants(); // Initial sortieren

  } catch (err) {
    console.error('Error loading encounter:', err);
    error.value = 'Failed to load encounter data.';
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
    combatant.initiative = parseInt(newInitiative, 10) || 0; // Stelle sicher, dass es eine Zahl ist
    sortCombatants();
  }
}

function handleApplyDamage(amount) {
  const combatant = selectedCombatant.value;
  if (combatant && amount > 0) {
    combatant.currentHp = Math.max(0, combatant.currentHp - amount); // HP nicht unter 0
  }
}

function handleApplyHealing(amount) {
  const combatant = selectedCombatant.value;
  if (combatant && amount > 0) {
    combatant.currentHp = Math.min(combatant.maxHp, combatant.currentHp + amount); // HP nicht über maxHp
  }
}

function handleAddCondition(conditionId) {
  const combatant = selectedCombatant.value; // Finde den aktuell ausgewählten
  if (combatant) {
    // Prüfe, ob die Bedingung nicht schon vorhanden ist
    if (!combatant.statusEffects.includes(conditionId)) {
      combatant.statusEffects.push(conditionId); // Füge die ID zum Array hinzu
      // Vue's Reaktivität sorgt dafür, dass CombatantItem aktualisiert wird
    }
    // Optional: Füge Logik hinzu, um eine Bedingung wieder zu entfernen
  }
}

function handleRemoveCondition(conditionId) {
  const combatant = selectedCombatant.value;
  if (combatant) {
    // Finde den Index der Bedingung im Array
    const index = combatant.statusEffects.indexOf(conditionId);
    if (index > -1) {
      // Entferne die Bedingung aus dem Array
      combatant.statusEffects.splice(index, 1);
    }
  }
}

function sortCombatants() {
  combatants.value.sort((a, b) => b.initiative - a.initiative); // Absteigend sortieren
}

// --- Watcher (Optional) ---
// Wenn sich die Combatant-Liste ändert (z.B. HP), wollen wir vielleicht die Auswahl beibehalten
watch(combatants, (newVal, oldVal) => {
  // Logik, falls nötig, z.B. wenn eine Kreatur auf 0 HP fällt etc.
}, { deep: true }); // 'deep' ist nötig, um Änderungen *innerhalb* der Objekte im Array zu erkennen

</script>

      
<template>
  <!-- v-container fluid sorgt weiterhin für den äußeren Abstand und volle Breite -->
  <v-container fluid>
    <v-row>
      <!-- Spalte 1: Encounter Auswahl & Initiative Liste -->
      <!-- md="5" nimmt 5/12 (ca. 41.7%, nahe 40%) der Breite auf md+ Screens -->
      <!-- cols="12" sorgt für volle Breite auf xs/sm Screens (Stacking) -->
      <v-col cols="12" md="5">
        <EncounterSelector @encounter-selected="handleEncounterSelected" />
        <v-divider class="my-4"></v-divider>

        <div v-if="isLoading">Loading Encounter... <v-progress-circular indeterminate></v-progress-circular></div>
        <v-alert v-if="error" type="error">{{ error }}</v-alert>

        <InitiativeList
          v-if="combatants.length > 0"
          :combatants="combatants"
          :selected-combatant-id="selectedCombatantId"
          @select-combatant="handleSelectCombatant"
          @update-initiative="handleUpdateInitiative"
        />
        <div v-else-if="!isLoading && !error && selectedEncounterId">
          No combatants in this encounter or encounter loaded incorrectly.
        </div>
         <div v-else-if="!isLoading && !error && !selectedEncounterId">
          Select an encounter to begin.
        </div>
      </v-col>

      <!-- Spalte 2: Interaktion -->
      <!-- md="3" nimmt 3/12 (exakt 25%) der Breite auf md+ Screens -->
      <v-col cols="12" md="3">
        <InteractionPanel
          :calculated-difficulty="encounterDifficulty"
          :selected-combatant-id="selectedCombatantId"
          :selected-combatant-conditions="selectedConditions"
          @apply-damage="handleApplyDamage"
          @apply-healing="handleApplyHealing"
          @add-condition="handleAddCondition"
          @remove-condition="handleRemoveCondition"
          
        />
      </v-col>

      <!-- Spalte 3: Stat Blocks -->
      <!-- md="4" nimmt 4/12 (ca. 33.3%, nahe 35%) der Breite auf md+ Screens -->
      <v-col cols="12" md="4">
         <StatBlockDisplay
           v-if="uniqueMonsterStatBlocks.length > 0"
           :unique-monster-stat-blocks="uniqueMonsterStatBlocks"
         />
         <div v-else-if="selectedEncounterId && !isLoading">
            No monster stats to display for this encounter.
         </div>
      </v-col>
    </v-row>
  </v-container>
</template>

<style scoped>
/* Füge hier bei Bedarf spezifische Stile hinzu */
</style>