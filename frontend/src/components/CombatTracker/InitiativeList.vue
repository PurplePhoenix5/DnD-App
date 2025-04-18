<script setup>
import CombatantItem from './CombatantItem.vue'; // Importiere die Kindkomponente

// --- Props ---
// defineProps deklariert die Daten, die diese Komponente von ihrer Elternkomponente erwartet.
const props = defineProps({
  combatants: { // Das Array der Kreaturen im Kampf
    type: Array,
    required: true,
    default: () => [] // Standardwert ist ein leeres Array
  },
  selectedCombatantId: { // Die ID der aktuell ausgewählten Kreatur (oder null)
    type: [String, null],
    default: null
  }
});

// --- Events ---
// Deklariert die Events, die diese Komponente aussenden kann.
const emit = defineEmits(['select-combatant', 'update-initiative']);

// --- Methoden (zum Weiterleiten von Events) ---
function handleSelectCombatant(combatantId) {
  // Leitet das Event von CombatantItem nach oben weiter
  emit('select-combatant', combatantId);
}

function handleUpdateInitiative(payload) {
  // Leitet das Event von CombatantItem nach oben weiter
  // payload ist ein Objekt, z.B. { combatantId: '...', newInitiative: 12 }
  emit('update-initiative', payload);
}
</script>

<template>
  <div>
    <h3 class="mb-2">Initiative Order</h3>
    <!-- Überprüfe, ob Combatants vorhanden sind -->
    <div v-if="combatants.length > 0">
      <!-- v-for iteriert über das 'combatants'-Array. -->
      <!-- Für jede Kreatur 'combatant' im Array wird eine CombatantItem-Komponente erstellt. -->
      <CombatantItem
        v-for="combatant in combatants"
        :key="combatant.id"          
        :combatant="combatant"       
        :is-selected="combatant.id === selectedCombatantId" 
        @select-combatant="handleSelectCombatant" 
        @update-initiative="handleUpdateInitiative" 
        class="mb-2"                 
      />
      <!-- 
        :key ist wichtig für Vue, um Listenelemente effizient zu verwalten.
        :combatant übergibt das Kreaturen-Objekt an die Kindkomponente.
        :is-selected übergibt, ob diese Kreatur die aktuell ausgewählte ist.
        @select-combatant lauscht auf das Event von CombatantItem und ruft handleSelectCombatant auf.
        @update-initiative lauscht auf das Event von CombatantItem und ruft handleUpdateInitiative auf.
        class="mb-2" fügt einen kleinen Abstand unter jedem Item hinzu.
      -->
    </div>
    <div v-else>
      <p>No combatants in the current encounter.</p>
    </div>
  </div>
</template>

<style scoped>
h3 {
  color: #eee; /* Beispiel: Kleine Stilanpassung */
}
</style>