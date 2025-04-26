<!-- frontend/src/components/MonsterCreator/MonsterCreator.vue -->
<script setup>
import { ref, reactive, watch, computed, onMounted } from 'vue';
import { set, get, isEqual, cloneDeep  } from 'lodash'; // Importiere set, get und isEqual von lodash
import { Splitpanes, Pane } from 'splitpanes';
import 'splitpanes/dist/splitpanes.css';
import { loadDnDData, preloadCommonData } from '../../utils/dndDataService.js';
import { calculateInitiativeBonus } from '../../utils/mathRendering.js';

import MonsterLoader from './MonsterLoader.vue';
import MonsterConfigurator from './MonsterConfigurator.vue';
import StatBlockRenderer from '../StatBlockRenderer.vue'; // Pfad prüfen

function handleMonsterFieldUpdate({ path, value }) {
    console.log(`MonsterCreator: Updating path "${path}" with value:`, value);
    set(monsterBeingCreated, path, value); // Update den Wert

    // --- Neuberechnungen auslösen ---

    // 1. Proficiency Bonus bei CR-Änderung
    if (path === 'basics.CR' && typeof value === 'number') {
        updateProficiencyFromCR(value);
        // Neuberechnung der Initiative wird durch PB-Änderung im Watch unten ausgelöst
    }

    // 2. Default HD Size bei Size-Änderung (wenn kein Override)
    if (path === 'basics.size' && get(monsterBeingCreated, 'basics.HP.overrideDie') === null) {
         const sizeVal = value; // Der neue Size-Wert
         const hdMapping = { 'Tiny': 4, 'Small': 6, 'Medium': 8, 'Large': 10, 'Huge': 12, 'Gargantuan': 20, 'Titan': 20 };
         const defaultDie = hdMapping[sizeVal] || 8;
         set(monsterBeingCreated, 'basics.HP.defaultDie', defaultDie);
    }

    // 3. Default Initiative bei relevanter Änderung (DEX, Prof, Exp) *nur wenn kein Override*
    if ((path.startsWith('basics.stats.DEX') || path === 'basics.Initiative.initProficiency' || path === 'basics.Initiative.initExpertise')
        && get(monsterBeingCreated, 'basics.Initiative.initOverrideValue') === null)
    {
         console.log("MonsterCreator: Triggering recalculateDefaultInitiative due to change in:", path);
         recalculateDefaultInitiative();
    }

     // 4. Wenn Override für Initiative entfernt wird, Default neu berechnen
     if (path === 'basics.Initiative.initOverrideValue' && value === null) {
          console.log("MonsterCreator: Triggering recalculateDefaultInitiative due to override removal");
          recalculateDefaultInitiative();
     }

    // Hier könnten weitere Berechnungen folgen (z.B. Saves bei Stat/PB-Änderung)
}

// Funktion zum Erstellen eines leeren Monsters (aktualisiert)
function createEmptyMonster() {
    const baseStats = { STR: 10, DEX: 10, CON: 10, INT: 10, WIS: 10, CHA: 10 };
    const baseSaves = {};
    Object.keys(baseStats).forEach(key => {
        baseSaves[key] = { proficient: false, defaultValue: 0, overrideValue: null };
    });
    const defaultInitiative = {
        initDefaultValue: calculateInitiativeBonus({ stats: baseStats, PB: 2 }, { initProficiency: false, initExpertise: false }), // Berechne Default Init
        initOverrideValue: null,
        initProficiency: false,
        initExpertise: false
    };

    return {
        complete: false,
        basics: {
            name: '', id: null, CR: null, PB: 2, size: 'Medium',
            HP: { defaultDie: 8, overrideDie: null, HDAmount: 1, HPmodifier: 0 },
            type: 'Humanoid', alignment: 'unaligned', languages: '',
            stats: { ...baseStats }, // Kopiere Basis-Stats
            AC: 10, ACType: '',
            Initiative: { ...defaultInitiative } // Kopiere Default Initiative
        },
        saves: cloneDeep(baseSaves), // Deep Clone für Saves
        speeds: [{ speed: 30, type: 'walk', note: '' }],
        skills: [],
        senses: {
            blindsight: 0, darkvision: 60, tremorsense: 0, truesight: 0,
            passivePerception: { defaultValue: 10, overrideValue: null },
            passiveInsight: { defaultValue: 10, overrideValue: null },
            sensesNotes: ''
        },
        resistances: [], immunities: [], vulnerabilities: [], conditionImmunities: [],
        inventory: '',
        traits: [],
        spellcasting: { /* ... wie zuvor ... */ },
        actions: { attackRoll: [], savingThrow: [], other: [] },
        bonusAction: { attackRoll: [], savingThrow: [], other: [] },
        multiattacks: '',
        reactions: [],
        legendaryActions: { uses: 0, usesInLair: 0, LegendaryResistanceUses: 0, legendaryResistanceType: '', legendaryAction: [] },
        lairActions: []
    };
}
// ========================================================================

const monsterBeingCreated = reactive(createEmptyMonster()); // Initialisiere mit leerem Template
const crDataList = ref([]); // Für PB Berechnung

const isLoadingMonsterDetails = ref(false);
const loadMonsterDetailsError = ref(null);
const isSaving = ref(false);
const saveError = ref(null);
const displayStyle = ref('2024');
const displayColumns = ref(1);

// --- Computed Properties (ID und Enabled State aktualisiert) ---
const monsterId = computed(() => {
    const name = monsterBeingCreated.basics?.name?.trim() || ''; // Sicherer Zugriff
    const crValue = monsterBeingCreated.basics?.CR;          // Sicherer Zugriff
    if (name && typeof crValue === 'number' && crValue >= 0) {
        return `${name.toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/^_|_$/g, '')}_cr${String(crValue).replace('.', '_')}`;
    }
    return null;
});

const isConfigurationEnabled = computed(() => {
    return !!monsterBeingCreated.basics?.name?.trim() && typeof monsterBeingCreated.basics?.CR === 'number' && monsterBeingCreated.basics.CR >= 0;
});
// ============================================================

// --- Methode zum Laden ---
async function handleLoadMonster(monsterIdToLoad) {
    if (!monsterIdToLoad) return;
    isLoadingMonsterDetails.value = true;
    loadMonsterDetailsError.value = null;
    try {
        const response = await fetch(`http://localhost:8080/api/monsters/${monsterIdToLoad}`);
        if (!response.ok) { throw new Error(`HTTP error! status: ${response.status}`); }
        const data = await response.json();

        // --- Aktualisiere das reactive Objekt ---
        // Verwende den Spread-Operator für eine flache Kopie und überschreibe dann
        // gezielt tiefere Strukturen oder füge Defaults hinzu.
        const loadedData = { ...createEmptyMonster(), ...data }; // Merge mit leerem Template als Basis

        // Kopiere die Werte in das reactive Objekt
        Object.keys(monsterBeingCreated).forEach(key => {
             if (loadedData[key] !== undefined) {
                 // Verwende _.isEqual für tiefen Vergleich bei Objekten/Arrays, um unnötige Updates zu vermeiden
                 if (typeof monsterBeingCreated[key] === 'object' && monsterBeingCreated[key] !== null) {
                     if (!isEqual(monsterBeingCreated[key], loadedData[key])) {
                         monsterBeingCreated[key] = _.cloneDeep(loadedData[key]); // Deep Clone für Objekte/Arrays
                     }
                 } else {
                      if (monsterBeingCreated[key] !== loadedData[key]) {
                         monsterBeingCreated[key] = loadedData[key];
                      }
                 }
             }
        });
         // Stelle sicher, dass ID gesetzt ist
        monsterBeingCreated.basics.id = monsterIdToLoad;


        console.log('MonsterCreator: Loaded monster:', JSON.parse(JSON.stringify(monsterBeingCreated)));
    } catch (err) { /* ... Fehlerbehandlung ... */
        console.error("Error loading monster details:", err);
        loadMonsterDetailsError.value = `Failed to load details for ${monsterIdToLoad}.`;
    } finally {
        isLoadingMonsterDetails.value = false;
    }
}
// ========================================================

// --- Update-Methode (aktualisiert) ---
function handleMonsterUpdate({ path, value }) {
    console.log(`MonsterCreator: Updating path "${path}" with value:`, value);
    // Verwende lodash.set für tief verschachtelte Updates
    set(monsterBeingCreated, path, value);

    // Spezielle Logik nach dem Update
    if (path === 'basics.CR' && typeof value === 'number') {
        updateProficiencyFromCR(value);
        // Nach CR Update auch Default Initiative neu berechnen (falls nicht overridden)
        if (monsterBeingCreated.basics?.Initiative?.initOverrideValue === null) {
            recalculateDefaultInitiative();
        }
    }
    // Wenn sich Stats oder Init-Proficiency/Expertise ändern, Default Init neu berechnen
    if ((path.startsWith('basics.stats.') || path.startsWith('basics.Initiative.initProf')) && monsterBeingCreated.basics?.Initiative?.initOverrideValue === null) {
         recalculateDefaultInitiative();
    }
    // Hier könnten weitere Berechnungen folgen (z.B. Save-Defaults)
}

// --- Proficiency Update (aktualisiert) ---
async function updateProficiencyFromCR(crValue) {
    if (crDataList.value.length === 0) {
        const data = await loadDnDData('crData.json');
        if (data) crDataList.value = data; else return;
    }
    const crInfo = crDataList.value.find(cr => cr.numeric === crValue);
    if (crInfo && monsterBeingCreated.basics) {
        const oldPB = monsterBeingCreated.basics.PB;
        monsterBeingCreated.basics.PB = crInfo.proficiency;
        console.log(`Proficiency updated to ${crInfo.proficiency} for CR ${crValue}`);
        // Nur neu berechnen, wenn PB sich geändert hat UND kein Override aktiv ist
        if (oldPB !== crInfo.proficiency && get(monsterBeingCreated, 'basics.Initiative.initOverrideValue') === null) {
             recalculateDefaultInitiative();
        }
    } else {
         console.warn(`No proficiency found for CR ${crValue}`);
         if (monsterBeingCreated.basics) monsterBeingCreated.basics.PB = 2; // Fallback
    }
}
// ============================================

// === NEU: Funktion zum Neuberechnen der Default Initiative ===
function recalculateDefaultInitiative() {
    if (!monsterBeingCreated.basics || !monsterBeingCreated.basics.Initiative) return;
    const initiativeConfig = {
        initProficiency: monsterBeingCreated.basics.Initiative.initProficiency,
        initExpertise: monsterBeingCreated.basics.Initiative.initExpertise
    };
    // Übergebe das basics Objekt, da es stats und PB enthält
    monsterBeingCreated.basics.Initiative.initDefaultValue = calculateInitiativeBonus(monsterBeingCreated.basics, initiativeConfig);
    console.log("Recalculated default initiative:", monsterBeingCreated.basics.Initiative.initDefaultValue);
}
// =============================================================

// --- Methode zum Speichern (angepasst) ---
async function saveMonster() {
    if (!isConfigurationEnabled.value) { /* ... Fehler ... */ return; }
    const finalMonsterId = monsterId.value;
    if (!finalMonsterId) { /* ... Fehler ... */ return; }
    // Stelle sicher, dass die ID jetzt im basics-Objekt ist
    monsterBeingCreated.basics.id = finalMonsterId;

    // TODO: Finale Validierung
    monsterBeingCreated.complete = true; // Annahme: gültig

    console.log("Saving Monster:", JSON.parse(JSON.stringify(monsterBeingCreated)));
    isSaving.value = true; /* ... */
    try {
        const response = await fetch(`http://localhost:8080/api/monsters/${finalMonsterId}`, { /* ... PUT Request ... */ });
        if (!response.ok) { /* ... Fehlerbehandlung ... */ }
        const savedData = await response.json();
        // Aktualisiere nach Speichern (wichtig, falls Backend Daten modifiziert)
        Object.keys(monsterBeingCreated).forEach(key => {
             if (savedData[key] !== undefined) { monsterBeingCreated[key] = savedData[key]; }
        });
        console.log("Monster saved/updated successfully!", savedData);
    } catch(err) { /* ... Fehlerbehandlung ... */
    } finally { isSaving.value = false; }
}

// --- Lifecycle Hook ---
onMounted(() => {
    preloadCommonData(); // Lädt DnDData
    loadDnDData('crData.json').then(data => { if(data) crDataList.value = data; });
});

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
    <!-- Splitpanes -->
    <splitpanes class="default-theme" style="height: calc(100vh - 200px);">
        <!-- Pane 1: Konfiguration -->
        <pane size="50">
            <div class="pa-2 bg-surface" style="height: 100%; overflow-y: auto;">
                 <MonsterConfigurator
                     :monster-data="monsterBeingCreated"
                     :is-enabled="isConfigurationEnabled"
                     @update-monster-field="handleMonsterFieldUpdate"
                 />
                 <v-btn
                    color="primary"
                    @click="saveMonster"
                    :loading="isSaving"
                    :disabled="isSaving || !isConfigurationEnabled"
                    class="mt-4"
                 >
                    Save Monster
                 </v-btn>
                 <v-alert v-if="saveError" type="error" density="compact" class="mt-2">{{ saveError }}</v-alert>
            </div>
        </pane>
        <!-- Pane 2: Statblock Vorschau -->
        <pane size="50">
           <div class="pa-2 bg-surface" style="height: 100%; overflow-y: auto;">
             <v-card variant="outlined">
               <v-card-title>Stat Block Preview</v-card-title>
               <div v-if="isLoadingMonsterDetails" class="text-center pa-5">
                  <v-progress-circular indeterminate color="primary"></v-progress-circular>
                  <p>Loading monster details...</p>
               </div>
               <v-card-text v-else>
                   <!-- === StatBlockRenderer auskommentiert === -->
                   <!--
                   <StatBlockRenderer
                       :monster-data="monsterBeingCreated"
                       :display-style="displayStyle"
                       :columns="displayColumns"
                   />
                   -->
                   <!-- === Platzhalter hinzugefügt === -->
                   <div class="pa-4 text-center text-disabled">
                       <p>Stat Block Preview is temporarily disabled.</p>
                       <p>Current Monster Data:</p>
                       <pre style="max-height: 400px; overflow: auto; text-align: left; background-color: rgba(0,0,0,0.1); padding: 8px; border-radius: 4px;">{{ JSON.stringify(monsterBeingCreated, null, 2) }}</pre>
                   </div>
                   <!-- ============================== -->
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