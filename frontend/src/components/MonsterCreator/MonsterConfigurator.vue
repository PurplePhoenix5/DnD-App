<!-- frontend/src/components/MonsterCreator/MonsterConfigurator.vue -->
<script setup>
import { ref } from 'vue';

// Daten für die Expansion Panels
const panels = ref([
  { id: 'basics', title: 'Basics', icon: 'mdi-clipboard-text-outline' },
  { id: 'saves', title: 'Saving Throws', icon: 'mdi-shield-half-full' },
  { id: 'speeds', title: 'Speeds', icon: 'mdi-run-fast' },
  { id: 'skills', title: 'Skills', icon: 'mdi-star-check-outline' },
  { id: 'senses', title: 'Senses', icon: 'mdi-eye-outline' },
  { id: 'resistances', title: 'Resistances & Immunities', icon: 'mdi-shield-check-outline' },
  { id: 'inventory', title: 'Inventory', icon: 'mdi-treasure-chest-outline' }, // Icon geändert
  { id: 'traits', title: 'Traits', icon: 'mdi-puzzle-outline' },
  { id: 'actions', title: 'Actions', icon: 'mdi-sword' },
  { id: 'bonusActions', title: 'Bonus Actions', icon: 'mdi-plus-circle-multiple-outline' }, // Icon geändert
  { id: 'reactions', title: 'Reactions', icon: 'mdi-reply' },
  { id: 'legendary', title: 'Legendary Actions', icon: 'mdi-crown-outline' },
  { id: 'lair', title: 'Lair Actions', icon: 'mdi-castle' },
]);

// Optional: Steuert, welche Panels standardmäßig geöffnet sind (z.B. das erste)
// const openPanels = ref(['basics']); // Öffnet das Panel mit value="basics"
</script>

<template>
  <!-- v-expansion-panels als Hauptcontainer -->
  <!-- 'variant="inset"' oder "accordion" oder "popout" für verschiedene Looks -->
  <!-- 'multiple' erlaubt mehrere geöffnete Panels gleichzeitig -->
  <v-expansion-panels
    variant="inset"
    multiple
    #default
    #v-model="openPanels" 
  >
    <!-- Iteriere durch die Panel-Daten -->
    <v-expansion-panel
      v-for="panel in panels"
      :key="panel.id"
      :value="panel.id"
      elevation="1" 
    >
      <!-- Titel des Panels mit Icon -->
      <v-expansion-panel-title>
        <v-icon :icon="panel.icon" start class="mr-2"></v-icon>
        {{ panel.title }}
      </v-expansion-panel-title>

      <!-- Inhalt des Panels (Platzhalter) -->
      <v-expansion-panel-text>
        <p class="text-disabled pa-4">
          Configuration for {{ panel.title }} goes here...
        </p>
        <!-- Hier kommen später die eigentlichen Formularfelder rein -->
      </v-expansion-panel-text>
    </v-expansion-panel>
  </v-expansion-panels>
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