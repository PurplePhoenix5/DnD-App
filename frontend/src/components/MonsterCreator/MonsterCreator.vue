<!-- frontend/src/components/MonsterCreator/MonsterCreator.vue -->
<script setup>
import { ref, reactive } from 'vue';
import MonsterConfigurator from './MonsterConfigurator.vue'; // Import aus demselben Ordner
// WICHTIG: Importiere den Renderer aus dem übergeordneten components-Ordner!
import StatBlockRenderer from '../StatBlockRenderer.vue';

// --- Reaktiver Zustand für das Monster ---
// 'reactive' eignet sich gut für komplexe, verschachtelte Objekte.
// Initialisiere mit einer minimalen Struktur, die der Renderer erwartet,
// oder mit den Werten eines "leeren" Monsters basierend auf deinem Schema.
const monsterBeingCreated = reactive({
  // --- Basis-Infos ---
  name: 'New Creature',
  saveVersion: 10, // Standardwert aus deinem Schema
  size: 'Medium',
  type: 'Humanoid',
  alignment: 'unaligned',
  AC: 10,
  ACType: 'natural armor',
  CR: 0, // Startwert CR
  proficiency: 2, // Typischerweise 2 für niedrige CRs
  // --- HP (Beispielstruktur) ---
  HP: {
    HD: 1,
    type: 8,
    modifier: 0
  },
  // --- Stats (Standard 10) ---
  stats: {
    STR: 10, DEX: 10, CON: 10, INT: 10, WIS: 10, CHA: 10
  },
  // --- Leere Arrays/Objekte für andere Sektionen ---
  speeds: [{ id: 'speed_walk', type: 'walk', speed: 30, note: '' }],
  saves: { // Initialisiere Saves basierend auf Stats (oder Default false)
      STR: { proficient: false, override: false, overrideValue: 0 },
      DEX: { proficient: false, override: false, overrideValue: 0 },
      CON: { proficient: false, override: false, overrideValue: 0 },
      INT: { proficient: false, override: false, overrideValue: 0 },
      WIS: { proficient: false, override: false, overrideValue: 0 },
      CHA: { proficient: false, override: false, overrideValue: 0 }
  },
  skills: [], // Beispiel: [{ key: 'PERCEPTION', skill: {stat: 'WIS', label: 'Perception'}, proficient: true, expertise: false, override: false, overrideValue: 0 }]
  resistances: [],
  immunities: [],
  vulnerabilities: [],
  conditions: [],
  senses: { blindsight: 0, darkvision: 0, tremorsense: 0, truesight: 0 },
  passivePerception: { override: false, overrideValue: 10 }, // Default
  languages: '',
  traits: [],
  actions: [],
  attacks: [],
  spellcasting: { // Leere Spellcasting-Struktur
      stat: 'INT',
      save: { override: false, overrideValue: 0 },
      modifier: { override: false, overrideValue: 0 },
      attack: { override: false, overrideValue: 0 },
      level: 1, slots: [0,0,0,0,0,0,0,0,0], atWill: [], standard: [], notes: '', atWillNotes: ''
   },
  multiattacks: [],
  multiattackOptions: {},
  legendaryActions: { count: 0, actions: [] },
  mythicActions: { triggerName: '', triggerRecharge: '', triggerDescription: '', preamble: '', actions: []},
  reactions: [],
  lairActions: [],
  regionalEffects: [],
  // ... füge weitere Felder aus deinem Schema mit Standardwerten hinzu ...
});

// --- Methoden (Platzhalter für später) ---
// Diese Funktionen würden durch Events von MonsterConfigurator aufgerufen werden
// function updateMonsterProperty(payload) {
//   // Beispiel: monsterBeingCreated[payload.key] = payload.value;
//   // Bei verschachtelten Objekten:
//   // monsterBeingCreated.stats[payload.stat] = payload.value;
// }
// function addMonsterAction(actionData) {
//    monsterBeingCreated.actions.push(actionData);
// }
// ... etc. ...

// --- Methode zum Speichern (Platzhalter für später) ---
async function saveMonster() {
  console.log("Saving Monster:", JSON.parse(JSON.stringify(monsterBeingCreated)));
  // TODO: Implement API call to POST/PUT monster data to backend
  // Der Backend-Endpunkt müsste das 5emm-v10 Format akzeptieren und
  // in eine entsprechende .json Datei im data/monsters Ordner speichern.
  alert("Save functionality not implemented yet.");
}

</script>

<template>
  <v-container fluid>
    <v-row>
      <!-- Spalte 1: Konfiguration -->
      <v-col cols="12" md="6">
        <MonsterConfigurator
           <!-- :monster="monsterBeingCreated" -->
           <!-- @update-property="updateMonsterProperty" -->
           <!-- @add-action="addMonsterAction" -->
           <!-- TODO: Props und Events hinzufügen, wenn Konfigurator implementiert ist -->
        />
        <!-- Speicher-Button (temporär hier, könnte auch im Configurator sein) -->
         <v-btn color="primary" @click="saveMonster" class="mt-4">
            Save Monster (Not Implemented)
         </v-btn>
      </v-col>

      <!-- Spalte 2: Statblock Vorschau -->
      <v-col cols="12" md="6">
        <v-card variant="outlined">
            <v-card-title>Stat Block Preview</v-card-title>
            <v-card-text>
                <!-- Übergebe das reaktive Monsterobjekt an den Renderer -->
                <StatBlockRenderer :monster-data="monsterBeingCreated" />
            </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<style scoped>
/* Optional: Stile für den Creator-Bereich */
</style>