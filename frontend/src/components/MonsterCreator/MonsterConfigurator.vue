<!-- frontend/src/components/MonsterCreator/MonsterConfigurator.vue -->
<script setup>
import { ref } from 'vue';
import { get } from 'lodash'; // Importiere get von lodash

import BasicsConfig from './Basics/BasicsConfig.vue';

const props = defineProps({
    monsterData: { type: Object, required: true },
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update-monster-field']);

function handleFieldUpdate(payload) { // payload ist { key, value }
    // Leite den Pfad und Wert einfach nach oben weiter
    // Füge den Panel-Pfad hinzu, falls nötig (hier ist es 'basics')
    emit('update-monster-field', { path: `basics.${payload.key}`, value: payload.value });
    // Für andere Panels wäre es z.B. `saves.${payload.key}`
}

// Daten für die Expansion Panels
const panels = ref([
  { id: 'basics', title: 'Basics', icon: 'mdi-clipboard-text-outline', path: 'basics', component: BasicsConfig },
  { id: 'saves', title: 'Saving Throws', icon: 'mdi-shield-half-full' },
  { id: 'speeds', title: 'Speeds', icon: 'mdi-run-fast' },
  { id: 'skills', title: 'Skills', icon: 'mdi-star-check-outline' },
  { id: 'senses', title: 'Senses', icon: 'mdi-eye-outline' },
  { id: 'resistances', title: 'Resistances & Immunities', icon: 'mdi-shield-check-outline' },
  { id: 'inventory', title: 'Inventory', icon: 'mdi-treasure-chest-outline' }, 
  { id: 'traits', title: 'Traits', icon: 'mdi-puzzle-outline' },
  { id: 'spellcaszing', title: 'Spellcasting', icon: 'mdi-magic-staff' },
  { id: 'actions', title: 'Actions', icon: 'mdi-sword' },
  { id: 'multiattack', title: 'Multi Attack', icon: 'mdi-plus-circle-multiple-outline' }, 
  { id: 'bonusactions', title: 'Bonus Actions', icon: 'mdi-sword-cross' },
  { id: 'reactions', title: 'Reactions', icon: 'mdi-reply' },
  { id: 'legendary', title: 'Legendary Actions', icon: 'mdi-crown-outline' },
  { id: 'lair', title: 'Lair Actions', icon: 'mdi-castle' },
]);

function handleUpdate(path, value) {
    emit('update-monster', { path, value });
}

// Verwende lodash.get, um die Daten sicher zu extrahieren
const getDataForPanel = (panel) => {
    if (!panel.path) return {}; // Fallback für Panels ohne Pfad
    if (panel.path === 'basics') {
         // Gib das gesamte basics-Objekt zurück
         return props.monsterData.basics ?? {}; // Fallback auf leeres Objekt
    }
    // Standard: Hole den Wert über den Pfad
    return get(props.monsterData, panel.path, {}); // Gib leeres Objekt als Default zurück
};

</script>

<template>
  <v-expansion-panels variant="inset" multiple #default #v-model="openPanels">
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
           :is-enabled="panel.id === 'basics' || isEnabled" 
           @update:field="handleFieldUpdate($event)"
        />
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