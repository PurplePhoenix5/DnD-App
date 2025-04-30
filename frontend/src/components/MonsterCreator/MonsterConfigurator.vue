<!-- frontend/src/components/MonsterCreator/MonsterConfigurator.vue -->
<script setup>
import { ref } from 'vue';
import { get } from 'lodash'; // Importiere get von lodash

import BasicsConfig from './Basics/BasicsConfig.vue';
import SavesConfig from './Saving Throws/SavingThrowsConfig.vue';
import SpeedsConfig from './Speeds/SpeedsConfig.vue';
import SkillsConfig from './Skills/SkillsConfig.vue';
import SensesConfig from './Senses/SensesConfig.vue';
import ResistancesAndImmunitiesConfig from './Resistances & Immunities/ResistancesAndImmunitiesConfig.vue';
import InventoryConfig from './Inventory/InventoryConfig.vue';
import TraitsConfig from './Traits/TraitsConfig.vue';

const props = defineProps({
    monsterData: { type: Object, required: true },
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update-monster-field']);

function handleFieldUpdate(payload) { // payload ist { key, value }
  console.log('MonsterConfigurator: Relaying update:', payload);  
  emit('update-monster-field', { path: payload.key, value: payload.value });
}

// Daten für die Expansion Panels
const panels = ref([
  { id: 'basics', title: 'Basics', icon: 'mdi-clipboard-text-outline', path: 'basics', component: BasicsConfig },
  { id: 'saves', title: 'Saving Throws', icon: 'mdi-shield-half-full', path: 'saves', component: SavesConfig },
  { id: 'speeds', title: 'Speeds', icon: 'mdi-run-fast', path: 'speeds', component: SpeedsConfig  },
  { id: 'skills', title: 'Skills', icon: 'mdi-star-check-outline', path: 'skills', component: SkillsConfig },
  { id: 'senses', title: 'Senses', icon: 'mdi-eye-outline', path: 'senses', component: SensesConfig },
  { id: 'resistances', title: 'Resistances & Immunities', icon: 'mdi-shield-check-outline', path: 'resistImmun', component: ResistancesAndImmunitiesConfig },
  { id: 'inventory', title: 'Inventory', icon: 'mdi-treasure-chest-outline', path: 'inventory', component: InventoryConfig }, 
  { id: 'traits', title: 'Traits', icon: 'mdi-puzzle-outline', path: 'traits', component: TraitsConfig },
  { id: 'spellcaszing', title: 'Spellcasting', icon: 'mdi-magic-staff' },
  { id: 'actions', title: 'Actions', icon: 'mdi-sword' },
  { id: 'multiattack', title: 'Multi Attack', icon: 'mdi-plus-circle-multiple-outline' }, 
  { id: 'bonusactions', title: 'Bonus Actions', icon: 'mdi-sword-cross' },
  { id: 'reactions', title: 'Reactions', icon: 'mdi-reply' },
  { id: 'legendary', title: 'Legendary Actions', icon: 'mdi-crown-outline' },
  { id: 'lair', title: 'Lair Actions', icon: 'mdi-castle' },
]);

// Verwende lodash.get, um die Daten sicher zu extrahieren
const getDataForPanel = (panel) => {
    if (!panel.path) return {}; // Fallback für Panels ohne Pfad
    if (panel.path === 'basics') {
         // Gib das gesamte basics-Objekt zurück
         return props.monsterData.basics ?? {}; // Fallback auf leeres Objekt
    }
    if (panel.path === 'skills') {
         return get(props.monsterData, 'skills', []); // Default leeres Array
    }
    if (panel.path === 'senses') {
         return get(props.monsterData, 'senses', {});
    }
    if (panel.path === 'resistImmun') {
         return {
             resistances: get(props.monsterData, 'resistances', []),
             immunities: get(props.monsterData, 'immunities', []),
             vulnerabilities: get(props.monsterData, 'vulnerabilities', []),
             conditionImmunities: get(props.monsterData, 'conditionImmunities', [])
         };
    }
     if (panel.path === 'inventory') {
          // Gib den String direkt zurück, nicht in ein Objekt packen
         return get(props.monsterData, 'inventory', '');
     }
     if (panel.path === 'traits') {
         return get(props.monsterData, 'traits', []);
    }
    // Standard: Hole den Wert über den Pfad
    return get(props.monsterData, panel.path, {}); // Gib leeres Objekt als Default zurück
};

</script>

<template>
  <v-expansion-panels variant="popout" multiple #default #v-model="openPanels">
    <v-expansion-panel 
      v-for="panel in panels"
      :key="panel.id"
      :value="panel.id"
      elevation="1"
      #default
      :disabled="panel.id !== 'basics' && !isEnabled"
    >
      <v-expansion-panel-title>
        <v-icon :icon="panel.icon" start class="mr-2"></v-icon>
        {{ panel.title }}
      </v-expansion-panel-title>

      <v-expansion-panel-text eager>
        <!-- Ansatz mit dynamischer Komponente (oder v-if wie zuvor) -->
        <component
           v-if="panel.component"
           :is="panel.component"
           :modelValue="getDataForPanel(panel)"
           :basicsData="getDataForPanel({ path: 'basics' })"
           :skillsData="getDataForPanel({ path: 'skills'})" 
           :allSkillsInfo="allSkillsData"                   
           :is-enabled="panel.id === 'basics' || isEnabled"
           @update:field="handleFieldUpdate($event)"
        />
        <SpeedsConfig v-else-if="panel.id === 'speeds'"
                      :modelValue="getDataForPanel(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <SkillsConfig v-else-if="panel.id === 'skills'"
                      :modelValue="getDataForPanel(panel)"
                      :basicsData="getDataForPanel({ path: 'basics' })" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
                      <SensesConfig v-else-if="panel.id === 'senses'"
                      :modelValue="getDataForPanel(panel)"
                      :basicsData="getDataForPanel({ path: 'basics' })" 
                      :skillsData="getDataForPanel({ path: 'skills' })" 
                      :allSkillsInfo="allSkillsData" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <ResistImmunConfig v-else-if="panel.id === 'resistances'"
                      :modelValue="getDataForPanel(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <InventoryConfig v-else-if="panel.id === 'inventory'"
                      :modelValue="getDataForPanel(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <TraitsConfig v-else-if="panel.id === 'traits'"
                      :modelValue="getDataForPanel(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />              
        <p v-else class="text-disabled pa-4">
          Configuration for {{ panel.title }} not implemented yet.
        </p>
      </v-expansion-panel-text>
    </v-expansion-panel>
  </v-expansion-panels>
  <div v-if="!isEnabled" class="text-disabled text-center mt-4 text-caption">
      Please enter Monster Name and CR in the "Basics" section to enable configuration.
  </div>
</template>

<style scoped>
.v-expansion-panel-text {
  /* Optional: Füge etwas Padding hinzu, wenn der Inhalt direkt im Text ist */
  /* padding: 16px; */
}
.v-expansion-panel-title {
   font-weight: 500; /* Etwas fetter Titel */
}
</style>