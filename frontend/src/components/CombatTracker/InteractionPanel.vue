<script setup>
import { ref, computed, nextTick } from 'vue';

// --- Props ---
const props = defineProps({
  encounterDifficultyProp: {
     type: String, 
     default: 'N/A' 
  },
  selectedCombatantId: {
    type: [String, null],
    default: null
  },
  selectedCombatantConditions: {
    type: Array, 
    default: () => [] }
});

// --- Events ---
const emit = defineEmits(['apply-damage', 'apply-healing', 'add-condition', 'remove-condition']);

// --- Lokale Daten ---
const amount = ref(1); // Der Wert im Eingabefeld, startet bei 1

const availableConditions = ref([
  { id: 'baned', name: 'Baned', icon: '/assets/conditions/baned.png' },
  { id: 'bleeding', name: 'Bleeding', icon: '/assets/conditions/bleeding.png' },
  { id: 'blinded', name: 'Blinded', icon: '/assets/conditions/blinded.png' },
  { id: 'charmed', name: 'Charmed', icon: '/assets/conditions/charmed.png' },
  { id: 'concentrating', name: 'Concentrating', icon: '/assets/conditions/concentrating.png' },
  { id: 'confused', name: 'Confused', icon: '/assets/conditions/confused.png' },
  { id: 'deafened', name: 'Deafened', icon: '/assets/conditions/deafened.png' },
  { id: 'entangled', name: 'Entangled', icon: '/assets/conditions/entangled.png' },
  { id: 'exhausted', name: 'Exhausted', icon: '/assets/conditions/exhausted.png' },
  { id: 'flying', name: 'Flying', icon: '/assets/conditions/flying.png' },
  { id: 'frightened', name: 'Frightened', icon: '/assets/conditions/frightened.png' },
  { id: 'grappled', name: 'Grappled', icon: '/assets/conditions/grappled.png' },
  { id: 'hex', name: 'Hex', icon: '/assets/conditions/hex.png' },
  { id: 'incapacitated', name: 'Incapacitated', icon: '/assets/conditions/incapacitated.png' },
  { id: 'invisible', name: 'Invisible', icon: '/assets/conditions/invisible.png' },
  { id: 'on fire', name: 'On Fire', icon: '/assets/conditions/on fire.png' },
  { id: 'paralyzed', name: 'Paralyzed', icon: '/assets/conditions/paralyzed.png' },
  { id: 'petrified', name: 'Petrified', icon: '/assets/conditions/petrified.png' },
  { id: 'poisoned', name: 'Poisoned', icon: '/assets/conditions/poisoned.png' },
  { id: 'prone', name: 'Prone', icon: '/assets/conditions/prone.png' },
  { id: 'restrained', name: 'Restrained', icon: '/assets/conditions/restrained.png' },
  { id: 'stunned', name: 'Stunned', icon: '/assets/conditions/stunned.png' },
  { id: 'unconscious', name: 'Unconscious', icon: '/assets/conditions/unconscious.png' },
  // Füge hier ggf. weitere Zustände hinzu (z.B. Exhaustion Level)
]);

// --- Computed Properties ---
// Bestimmt, ob die Buttons aktiv sein sollen
const canInteract = computed(() => {
  // Man braucht eine ausgewählte Kreatur UND einen positiven Wert im Feld
  return props.selectedCombatantId !== null && amount.value > 0;
});

const canModifyCondition = computed(() => props.selectedCombatantId !== null);

function isConditionActive(conditionId) {
  // Prüft, ob die conditionId im übergebenen Array ist
  return props.selectedCombatantConditions.includes(conditionId);
}

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

function toggleCondition(conditionId) {
  if (canModifyCondition.value) {
    if (isConditionActive(conditionId)) {
      // Bedingung ist aktiv -> Entfernen-Event senden
      emit('remove-condition', conditionId);
    } else {
      // Bedingung ist inaktiv -> Hinzufügen-Event senden
      emit('add-condition', conditionId);
    }
  }
}

async function handleAmountFocus(event) {
  // Wir verwenden event.target, um das tatsächliche <input>-Element zu bekommen,
  // das den Fokus erhalten hat. Das ist oft zuverlässiger als über die Ref
  // auf das Vuetify-Komponenten-Objekt zu gehen.
  const inputElement = event.target;

  if (inputElement && typeof inputElement.select === 'function') {
    // Warte auf den nächsten DOM-Update-Tick, bevor select() aufgerufen wird.
    // Dies stellt sicher, dass der Fokus-Event vollständig verarbeitet wurde
    // und die Auswahl zuverlässig funktioniert.
    await nextTick();
    inputElement.select();
  }
}

</script>

<template>
  <v-card>
    <v-card-text>
      <!-- Zeige Encounter Schwierigkeit -->
      <div class="mb-4">
        <span>Encounter Difficulty: </span>
        <v-chip :color="getDifficultyColor(encounterDifficultyProp)" size="small">
          {{ encounterDifficultyProp }}
        </v-chip>
      </div>

      <!-- Eingabefeld für Schaden/Heilung -->
      <v-text-field
        ref="amountInputRef"     
        v-model.number="amount"
        label="Amount"
        type="number"
        min="1"                 
        required                
        density="compact"
        variant="outlined"
        :disabled="!selectedCombatantId" 
        hide-details="auto"
        @focus="handleAmountFocus"
      ></v-text-field>
      <!-- :disabled: Feld ist deaktiviert, wenn keine Kreatur ausgewählt ist -->

      <!-- Knöpfe für Aktionen -->
      <div class="d-flex justify-space-around mt-4">
        <v-btn
          color="#2E7D32"
          @click="applyHealing"
          :disabled="!canInteract" 
          prepend-icon="mdi-heart-plus" 
        >
          Heal
        </v-btn>
        <v-btn
          color="#D50000"
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


        <!-- === NEU: Abschnitt für Conditions === -->
        <v-divider class="my-4"></v-divider>
        <h4 class="mb-2">Toggle Condition</h4>
        <div v-if="!canModifyCondition" class="text-caption text-center mb-2">
          (Select a combatant first)
        </div>
        <!-- Container für die Condition Icons/Namen -->
        <div class="d-flex flex-wrap ga-2"> <!-- ga-2 fügt Lücken hinzu -->
          <div
            v-for="condition in availableConditions"
            :key="condition.id"
            class="condition-item text-center"
            :class="{ 
              'disabled': !canModifyCondition,
              'active-condition': isConditionActive(condition.id) 
            }"
            @click="toggleCondition(condition.id)"
          >
            <!-- v-img zum Anzeigen der PNGs -->
            <v-img
              :src="condition.icon"
              :alt="condition.name"
              width="32"  
              height="32" 
              contain     
              class="condition-icon mx-auto" 
            ></v-img>
            <span class="text-caption">{{ condition.name }}</span>
          </div>
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
    default: return 'grey-darken-1';
  }
}
</script>

<style scoped>
.condition-item {
  cursor: pointer;
  padding: 4px;
  border-radius: 4px;
  transition: background-color 0.2s ease-in-out;
}
.condition-item:hover:not(.disabled) {
  background-color: rgba(var(--v-theme-on-surface), 0.1); /* Leichter Hover-Effekt */
}
.condition-item.disabled {
  cursor: not-allowed;
  opacity: 0.5;
}
.condition-icon {
  display: block; /* Stellt sicher, dass mx-auto funktioniert */
  margin-bottom: 2px;
}
.condition-item.active-condition {
  background-color: rgba(var(--v-theme-primary), 0.2); 
  border: 1px solid rgba(var(--v-theme-primary), 0.5);
}
</style>