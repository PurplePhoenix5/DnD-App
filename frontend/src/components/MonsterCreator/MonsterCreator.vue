<!-- frontend/src/components/MonsterCreator/MonsterCreator.vue -->
<script setup>
import { ref, reactive, watch, computed, onMounted } from 'vue';
import { set, get, isEqual, cloneDeep } from 'lodash';
import { Splitpanes, Pane } from 'splitpanes';
import 'splitpanes/dist/splitpanes.css';
import { loadDnDData, preloadCommonData } from '../../utils/dndDataService.js';
import { calculateInitiativeBonus, calculateSkillBonus, statModifier, saveModifierForStat } from '../../utils/mathRendering.js';

import MonsterLoader from './MonsterLoader.vue';
import MonsterConfigurator from './MonsterConfigurator.vue';
import ActionsConfig from './Actions/ActionsConfig.vue'; // Import Actions
import BonusActionsConfig from './Bonus Actions/BonusActionsConfig.vue'; // Import Bonus Actions
// import StatBlockRenderer from '../StatBlockRenderer.vue'; // Pfad prüfen

const emptyMonsterStructure = createEmptyMonster(); 
const monsterLoaderRef = ref(null);
const skillsDataGlobal = ref({}); 

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
        actions: [], // Initialisiere als leeres ARRAY
        bonusAction: [], // Initialisiere als leeres ARRAY
        multiattacks: '',
        reactions: [],
        legendaryActions: { uses: 0, usesInLair: 0, LegendaryResistanceUses: 0, legendaryResistanceType: '', legendaryAction: [] },
        lairActions: []
    };
}

function handleMonsterFieldUpdate({ path, value }) {
    console.log(`MonsterCreator: Received update - Path: "${path}", Value:`, value);

    let targetPath = path;
    if (emptyMonsterStructure.basics && path in emptyMonsterStructure.basics) {
        targetPath = `basics.${path}`;
    } else if (path === 'actions' || path === 'bonusAction' || path === 'traits') {
        console.log(`MonsterCreator: Explicitly assigning new array for path: ${path}`);
         // Stelle sicher, dass monsterBeingCreated diese Property hat
         if (monsterBeingCreated[path]) {
              monsterBeingCreated[path] = [...value]; // Nutze Spread für Array-Kopie
         } else {
              // Handle Fall, falls die Property noch nicht existiert (sollte nicht passieren bei createEmptyMonster)
              console.warn(`MonsterCreator: Attempted to update non-existent array path: ${path}`);
         };
    } else if (emptyMonsterStructure.basics?.HP && path.startsWith('HP.')) {
        targetPath = `basics.${path}`;
    } else if (emptyMonsterStructure.basics?.stats && path.startsWith('stats.')) {
        targetPath = `basics.${path}`;
    } else if (emptyMonsterStructure.basics?.Initiative && path.startsWith('Initiative.')) {
        targetPath = `basics.${path}`;
    }

    console.log(`MonsterCreator: Setting path "${targetPath}" with value:`, value);

    const oldPB = get(monsterBeingCreated, 'basics.PB'); 
    const oldStats = cloneDeep(get(monsterBeingCreated, 'basics.stats'));
    const pbAfterUpdate = get(monsterBeingCreated, 'basics.PB'); // PB *nach* möglicher CR-Änderung

    set(monsterBeingCreated, targetPath, value);
    let recalculateSaves = false;
    let recalculatePassives = false;

    if (path === 'basics.CR' && typeof value === 'number') {
         updateProficiencyFromCR(value); 
         recalculateSaves=true; 
        }
    if (path === 'basics.size' && get(monsterBeingCreated, 'basics.HP.overrideDie') ) { 
        const sizeVal = value;
        const hdMapping = { 'Tiny': 4, 'Small': 6, 'Medium': 8, 'Large': 10, 'Huge': 12, 'Gargantuan': 20, 'Titan': 20 };
        const defaultDie = hdMapping[sizeVal] || 8;
        set(monsterBeingCreated, 'basics.HP.defaultDie', defaultDie); 
    }
    if (path === 'skills' || path.startsWith('skills[')) { // Wenn das Array selbst oder ein Element geändert wird
        recalculatePassives = true;
        recalculateAllSaveDefaults(); // Auch Saves neu berechnen, falls Skills Proficiency teilen (unwahrscheinlich, aber sicher)
    }
    if (path === 'traits') {
        console.log("MonsterCreator: Explicitly assigning new traits array");
        monsterBeingCreated.traits = [...value];
    }
    if (path.startsWith('basics.stats.')) { 
        recalculateSaves=true; 
        recalculatePassives=true; 
    } 
    if ((targetPath === 'basics.stats.DEX' || targetPath === 'basics.Initiative.initProficiency' || targetPath === 'basics.Initiative.initExpertise' || (targetPath === 'basics.CR' && oldPB !== pbAfterUpdate) ) && get(monsterBeingCreated, 'basics.Initiative.initOverrideValue') === null) { 
        console.log("MonsterCreator: Triggering recalculateDefaultInitiative due to change in:", targetPath);
        recalculateDefaultInitiative(); 
    }
    if (targetPath === 'basics.Initiative.initOverrideValue' && value === null) { 
        console.log("MonsterCreator: Triggering recalculateDefaultInitiative due to override removal");
        recalculateDefaultInitiative();
     }
    if (path.startsWith('saves.')) { 
        recalculateSaves = true; 
    } // Save Prof/Override geändert
    if (path.startsWith('skills.')) { 
        recalculatePassives = true; 
    }
    if (targetPath.startsWith('basics.stats.') || targetPath.startsWith('saves.') || (targetPath === 'basics.CR' && oldPB !== pbAfterUpdate)) {
        recalculateAllSaveDefaults(); 
    }

    if (recalculateSaves) recalculateAllSaveDefaults();
    if (recalculatePassives) recalculatePassiveDefaults();

}

function recalculateAllSaveDefaults() {
     if (!monsterBeingCreated.basics || !monsterBeingCreated.saves) return;
     console.log("MonsterCreator: Recalculating all save defaults...");
     for (const statKey in monsterBeingCreated.saves) {
          // Berechne neuen Default nur, wenn KEIN Override aktiv ist
          if (monsterBeingCreated.saves[statKey].overrideValue === null) {
              const tempMonster = { basics: monsterBeingCreated.basics, saves: monsterBeingCreated.saves };
              const newDefault = saveModifierForStat(tempMonster, statKey); // Nutze mathRendering
              // Update direkt im Objekt
              monsterBeingCreated.saves[statKey].defaultValue = newDefault;
          }
     }
     console.log("MonsterCreator: Updated saves defaults:", JSON.parse(JSON.stringify(monsterBeingCreated.saves)));
}

function recalculatePassiveDefaults() {
    // Prüfe, ob die benötigten Daten (basics, skills, senses UND die geladenen skillsData) vorhanden sind
    if (!monsterBeingCreated.basics || !monsterBeingCreated.skills || !monsterBeingCreated.senses || Object.keys(skillsDataGlobal.value).length === 0) {
        console.log("MonsterCreator: Skipping passive default calculation - data not ready.");
        return;
    }
    console.log("MonsterCreator: Recalculating passive defaults...");

    // Passive Perception
    const ppConfig = monsterBeingCreated.senses.passivePerception;
    if (ppConfig && (ppConfig.overrideValue === null || ppConfig.overrideValue === undefined)) {
         const perceptionSkillInfo = monsterBeingCreated.skills.find(s => s.skill === 'Perception');
         // Rufe die importierte Funktion auf, übergebe globale Skill-Daten
         const perceptionBonus = perceptionSkillInfo
             ? calculateSkillBonus(monsterBeingCreated.basics, perceptionSkillInfo, skillsDataGlobal.value)
             : statModifier(monsterBeingCreated.basics.stats?.WIS); // Verwende importierte Funktion
         const newPPDefault = 10 + perceptionBonus;
         if (ppConfig.defaultValue !== newPPDefault) {
             set(monsterBeingCreated, 'senses.passivePerception.defaultValue', newPPDefault);
         }
    }

     // Passive Insight
     const piConfig = monsterBeingCreated.senses.passiveInsight;
     if (piConfig && (piConfig.overrideValue === null || piConfig.overrideValue === undefined)) {
          const insightSkillInfo = monsterBeingCreated.skills.find(s => s.skill === 'Insight');
          // Rufe die importierte Funktion auf
          const insightBonus = insightSkillInfo
              ? calculateSkillBonus(monsterBeingCreated.basics, insightSkillInfo, skillsDataGlobal.value)
              : statModifier(monsterBeingCreated.basics.stats?.WIS); // Verwende importierte Funktion
          const newPIDefault = 10 + insightBonus;
           if (piConfig.defaultValue !== newPIDefault) {
              set(monsterBeingCreated, 'senses.passiveInsight.defaultValue', newPIDefault);
           }
     }
      console.log("MonsterCreator: Updated passive defaults:", JSON.parse(JSON.stringify(monsterBeingCreated.senses)));
}
// ========================================================================

const monsterBeingCreated = reactive(createEmptyMonster()); // Initialisiere mit leerem Template
const crDataList = ref([]); // Für PB Berechnung

const isLoadingMonsterDetails = ref(false);
const loadMonsterDetailsError = ref(null);
const isSaving = ref(false);
const saveError = ref(null);
const isDeleting = ref(false);
const deleteError = ref(null);
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
                 if (typeof monsterBeingCreated[key] === 'object' && monsterBeingCreated[key] !== null && typeof loadedData[key] === 'object' && loadedData[key] !== null) { // Sicherere Prüfung
                     if (!isEqual(monsterBeingCreated[key], loadedData[key])) { // Verwende isEqual direkt
                         monsterBeingCreated[key] = cloneDeep(loadedData[key]); // Verwende cloneDeep direkt
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

        if (monsterLoaderRef.value) {
            monsterLoaderRef.value.refreshList();
        }

    } catch (err) { /* ... Fehlerbehandlung ... */
        console.error("Error loading monster details:", err);
        loadMonsterDetailsError.value = `Failed to load details for ${monsterIdToLoad}.`;
    } finally {
        isLoadingMonsterDetails.value = false;
    }
}
// ========================================================

// === Methode zum Speichern (wird jetzt vom Event aufgerufen) ===
async function handleSaveMonsterRequest() {
  if (!isConfigurationEnabled.value) { /* ... Fehler ... */ return; }
  const finalMonsterId = monsterId.value; // ID aus Computed holen
  if (!finalMonsterId) { /* ... Fehler ... */ return; }

  // --- Validierung vor dem Speichern ---
  let complete = true;
  // Beispiel-Validierung (passe an deine Mindestanforderungen an!)
  if (!monsterBeingCreated.basics.name?.trim()) complete = false;
  if (typeof monsterBeingCreated.basics.CR !== 'number' || monsterBeingCreated.basics.CR < 0) complete = false;
  if (!monsterBeingCreated.basics.type) complete = false;
  if (!monsterBeingCreated.basics.alignment) complete = false;
  if (!monsterBeingCreated.basics.HP || monsterBeingCreated.basics.HP.HDAmount < 1) complete = false;
  if (!monsterBeingCreated.basics.AC || monsterBeingCreated.basics.AC < 0) complete = false;
  if (!monsterBeingCreated.basics.stats || Object.values(monsterBeingCreated.basics.stats).some(s => typeof s !== 'number')) complete = false;
  // Füge weitere Prüfungen hinzu (z.B. mindestens eine Action/Attack?)

  monsterBeingCreated.complete = complete;
  monsterBeingCreated.basics.id = finalMonsterId; // Stelle sicher, dass ID gesetzt ist

  console.log("Saving Monster:", JSON.parse(JSON.stringify(monsterBeingCreated)));
  isSaving.value = true;
  saveError.value = null;
  deleteError.value = null; // Lösche alten Delete-Error

  // Bestimme den Zielordner basierend auf 'complete'
  const savePathSegment = complete ? 'completed' : 'uncompleted';
  // !!! WICHTIG: Das Backend muss diesen Pfad unterstützen !!!
  // Wir senden ihn hier nicht direkt, sondern das Backend entscheidet anhand des Flags.
  // Alternativ: Eigene API-Endpunkte /api/monsters/completed und /api/monsters/uncompleted

  try {
      // Verwende PUT /api/monsters/{id} für Upsert
      const response = await fetch(`http://localhost:8080/api/monsters/${finalMonsterId}`, {
           method: 'PUT',
           headers: { 'Content-Type': 'application/json' },
           body: JSON.stringify(monsterBeingCreated) // Sende das komplette Objekt
      });
       if (!response.ok) {
           const errorData = await response.json().catch(() => ({ message: 'Unknown error during save' }));
           throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || 'Save failed'}`);
       }
       const savedData = await response.json();
       // Aktualisiere lokalen State mit Antwort vom Server
       Object.assign(monsterBeingCreated, savedData);
       console.log("Monster saved/updated successfully!", savedData);
       // Optional: Erfolgsmeldung
       if (monsterLoaderRef.value) {
            monsterLoaderRef.value.refreshList(); // Rufe die exponierte Methode auf
            monsterLoaderRef.value.clearSelection(); // Rufe die exponierte Methode auf
       }

  } catch(err) {
      console.error("Error saving monster:", err);
      saveError.value = err.message || "Failed to save monster.";
  } finally {
       isSaving.value = false;
  }
}

// === NEU: Methode zum Löschen ===
async function handleDeleteMonsterRequest(monsterIdToDelete) {
    if (!monsterIdToDelete) {
        deleteError.value = "No monster selected to delete.";
        return;
    }
    // Bestätigung ist bereits im Loader erfolgt

    console.log(`Deleting Monster with ID: ${monsterIdToDelete}`);
    isDeleting.value = true;
    deleteError.value = null;
    saveError.value = null; // Lösche alten Save-Error

    try {
        const response = await fetch(`http://localhost:8080/api/monsters/${monsterIdToDelete}`, {
            method: 'DELETE'
        });
         if (!response.ok) {
             // Versuche Fehlermeldung zu bekommen
             let errorMsg = `Delete failed with status ${response.status}`;
             try {
                 const errorData = await response.json();
                 errorMsg = errorData.error || errorMsg;
             } catch (e) { /* Ignoriere JSON Parse Fehler bei Fehler */ }
             throw new Error(errorMsg);
         }
         console.log("Monster deleted successfully!");
         // Erfolgreich gelöscht -> Formular leeren und Monsterliste neu laden
         Object.assign(monsterBeingCreated, createEmptyMonster()); // Setze zurück
         // Lade Monsterliste im Loader neu (indem wir den Loader selbst neu laden oder ein Event senden)
         // Einfachste Variante: Gehe davon aus, dass fetchExistingMonsters im Loader neu getriggert wird,
         // wenn sich z.B. die Route ändert (was hier nicht der Fall ist).
         // Besser: Ein Event zum Neuladen senden oder den Loader Key ändern.
         // Workaround: Manuell die Liste im Loader aktualisieren (nicht ideal)
         // Alternative: Direkt hier die Liste im Loader neu fetchen (bricht Kapselung)

         // Zeige Erfolgsmeldung (z.B. Snackbar)

    } catch(err) {
         console.error("Error deleting monster:", err);
         deleteError.value = err.message || "Failed to delete monster.";
    } finally {
        isDeleting.value = false;
    }
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
onMounted(async () => {
    preloadCommonData(); 
    loadDnDData('crData.json').then(data => { if(data) crDataList.value = data; });
    const [crData, skillsData] = await Promise.all([
         loadDnDData('crData.json'),
         loadDnDData('skills.json')
    ]);
    if(crData) crDataList.value = crData;
    if(skillsData) skillsDataGlobal.value = skillsData; 

    recalculateAllSaveDefaults();
    recalculatePassiveDefaults();
});

</script>

<template>
    <v-container fluid>
      <!-- Monster Loader: Übergibt Status und hört auf neue Events -->
      <v-row>
          <v-col cols="12">
              <MonsterLoader
                  ref="monsterLoaderRef"
                  @load-monster="handleLoadMonster"
                  v-model:style="displayStyle"
                  v-model:columns="displayColumns"
                  :monster-name="monsterBeingCreated.basics?.name" 
                  :monster-c-r="monsterBeingCreated.basics?.CR" 
                  :is-saving="isSaving"
                  :is-deleting="isDeleting"
                  @save-monster="handleSaveMonsterRequest"
                  @delete-monster="handleDeleteMonsterRequest"
               />
               <v-alert v-if="loadMonsterDetailsError" type="error" density="compact">{{ loadMonsterDetailsError }}</v-alert>
          </v-col>
      </v-row>
  
      <!-- Splitpanes -->
      <splitpanes class="default-theme" style="height: calc(100vh - 200px);">
          <!-- Pane 1: Konfiguration -->
          <pane size="70">
              <div class="pa-2 bg-surface" style="height: 100%; overflow-y: auto;">
                   <MonsterConfigurator
                       :monster-data="monsterBeingCreated"
                       :is-enabled="isConfigurationEnabled"
                       :all-skills-info="skillsDataGlobal"
                       @update-monster-field="handleMonsterFieldUpdate"
                   />
                   <!-- Speicher-Button wurde in den Loader verschoben -->
                   <!-- <v-btn ... @click="handleSaveMonsterRequest" ... >Save Monster</v-btn> -->
                   <v-alert v-if="saveError" type="error" density="compact" class="mt-2">{{ saveError }}</v-alert>
                   <v-alert v-if="deleteError" type="error" density="compact" class="mt-2">{{ deleteError }}</v-alert> <!-- Delete Error anzeigen -->
              </div>
          </pane>
        <!-- Pane 2: Statblock Vorschau -->
        <pane size="30">
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
  background-color: #000000;
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