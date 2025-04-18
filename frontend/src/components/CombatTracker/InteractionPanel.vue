<script setup>
import { ref, computed } from 'vue';

// --- Props ---
const props = defineProps({
  calculatedDifficulty: {
    type: String,
    default: 'N/A'
  },
  selectedCombatantId: {
    type: [String, null],
    default: null
  }
});

// --- Events ---
const emit = defineEmits(['apply-damage', 'apply-healing']);

// --- Lokale Daten ---
const amount = ref(1); // Der Wert im Eingabefeld, startet bei 1

// --- Computed Properties ---
// Bestimmt, ob die Buttons aktiv sein sollen
const canInteract = computed(() => {
  // Man braucht eine ausgewählte Kreatur UND einen positiven Wert im Feld
  return props.selectedCombatantId !== null && amount.value > 0;
});

// --- Methoden ---
function applyDamage() {
  if (canInteract.value) {
    emit('apply-damage', amount.value);
  }
}

function applyHealing() {
  if (canInteract.value) {
    emit('apply-healing', amount.value);
  }
}

</script>

<template>
  <v-card>
    <v-card-title>Interaction</v-card-title>
    <v-card-text>
      <!-- Zeige Encounter Schwierigkeit -->
      <div class="mb-4">
        <span>Encounter Difficulty: </span>
        <v-chip :color="getDifficultyColor(calculatedDifficulty)" size="small">
          {{ calculatedDifficulty }}
        </v-chip>
      </div>

      <!-- Eingabefeld für Schaden/Heilung -->
      <v-text-field
        v-model.number="amount"
        label="Amount"
        type="number"
        min="1"                 
        required                
        density="compact"
        variant="outlined"
        :disabled="!selectedCombatantId" 
        hide-details="auto"
      ></v-text-field>
      <!-- :disabled: Feld ist deaktiviert, wenn keine Kreatur ausgewählt ist -->

      <!-- Knöpfe für Aktionen -->
      <div class="d-flex justify-space-around mt-4">
        <v-btn
          color="warning"
          @click="applyHealing"
          :disabled="!canInteract" 
          prepend-icon="mdi-heart-plus" 
        >
          Heal
        </v-btn>
        <v-btn
          color="error"
          @click="applyDamage"
          :disabled="!canInteract" 
          prepend-icon="mdi-sword"   
        >
          Damage
        </v-btn>
        <!-- :disabled="!canInteract": Knöpfe sind deaktiviert, wenn keine Kreatur ausgewählt oder der Wert <= 0 ist -->
        <!-- prepend-icon: Fügt ein Icon vor dem Text hinzu (benötigt @mdi/font) -->
      </div>

       <div v-if="!selectedCombatantId" class="text-caption text-center mt-4">
         Select a combatant in the list to apply damage or healing.
       </div>

    </v-card-text>
  </v-card>
</template>

<script>
// Hilfsfunktion für die Chip-Farbe (außerhalb von <script setup>)
function getDifficultyColor(difficulty) {
  switch (difficulty?.toLowerCase()) {
    case 'trivial': return 'grey';
    case 'easy': return 'success';
    case 'medium': return 'info';
    case 'hard': return 'warning';
    case 'deadly': return 'error';
    default: return 'grey';
  }
}
</script>

<style scoped>
/* Optional: CSS-Anpassungen */
</style>