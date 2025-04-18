<script setup>
import { ref, computed, watch } from 'vue';

// --- Props ---
const props = defineProps({
  combatant: {
    type: Object,
    required: true
  },
  isSelected: {
    type: Boolean,
    default: false
  }
});

// --- Events ---
const emit = defineEmits(['select-combatant', 'update-initiative']);

// --- Lokale Daten für Initiative-Bearbeitung ---
// Wir brauchen eine lokale Kopie, damit das Ändern im Textfeld nicht sofort
// die Sortierung auslöst, sondern erst, wenn der Benutzer fertig ist (z.B. bei @blur).
const localInitiative = ref(props.combatant.initiative);

// --- Computed Properties ---
// Berechnet den HP-Prozentsatz für die Progressionsleiste
const hpPercentage = computed(() => {
  if (!props.combatant.maxHp || props.combatant.maxHp <= 0) return 0;
  return (props.combatant.currentHp / props.combatant.maxHp) * 100;
});

// Bestimmt die Farbe der HP-Leiste basierend auf dem Prozentsatz
const hpColor = computed(() => {
  if (hpPercentage.value > 50) return 'success'; // Grün
  if (hpPercentage.value > 25) return 'warning'; // Gelb/Orange
  return 'error'; // Rot
});

// --- Methoden ---
function selectThisCombatant() {
  // Sendet die ID dieser Kreatur nach oben, wenn darauf geklickt wird
  emit('select-combatant', props.combatant.id);
}

function submitInitiativeChange() {
  // Wird aufgerufen, wenn das Initiative-Feld den Fokus verliert (@blur)
  // oder wenn Enter gedrückt wird (@keydown.enter)
  const newInitiative = parseInt(localInitiative.value, 10);
  if (!isNaN(newInitiative) && newInitiative !== props.combatant.initiative) {
    // Nur senden, wenn es eine gültige Zahl ist und sich der Wert geändert hat
    emit('update-initiative', {
      combatantId: props.combatant.id,
      newInitiative: newInitiative
    });
  } else {
    // Setze das lokale Feld zurück auf den Prop-Wert, falls die Eingabe ungültig war oder gleich blieb
    localInitiative.value = props.combatant.initiative;
  }
}

// --- Watcher ---
// Wenn sich die Initiative von außen ändert (z.B. durch Laden eines neuen Encounters),
// aktualisiere das lokale Textfeld.
watch(() => props.combatant.initiative, (newVal) => {
  localInitiative.value = newVal;
});

</script>

<template>
  <!-- v-card: Dient als Container für die Kreatur -->
  <!-- :variant="isSelected ? 'tonal' : 'elevated'": Ändert das Aussehen, wenn ausgewählt -->
  <!-- @click="selectThisCombatant": Ruft die Methode auf, wenn auf die Karte geklickt wird -->
  <v-card
    :variant="isSelected ? 'tonal' : 'elevated'"
    :color="isSelected ? 'primary' : undefined"
    @click="selectThisCombatant"
    density="compact"
  >
    <v-card-text>
      <v-row align="center" no-gutters>
        <!-- Spalte 1: Initiative -->
        <v-col cols="2">
          <!-- v-text-field für Initiative-Eingabe -->
          <v-text-field
            v-model.number="localInitiative"
            type="number"
            density="compact"
            hide-details
            variant="outlined"
            @blur="submitInitiativeChange"     
            @keydown.enter="submitInitiativeChange" 
            @click.stop                   
            aria-label="Initiative"
            class="initiative-input"
          />
          <!--
            @blur: Event wird ausgelöst, wenn das Feld den Fokus verliert.
            @keydown.enter: Event wird ausgelöst, wenn Enter gedrückt wird.
            @click.stop: Verhindert, dass der Klick auf das Feld auch die Karte auswählt (stoppt Event-Propagation).
          -->
        </v-col>

        <!-- Spalte 2: Name, AC, CR -->
        <v-col cols="6" class="pl-2">
          <div class="font-weight-bold">{{ combatant.name }}</div>
          <div class="text-caption">
            AC: {{ combatant.ac ?? 'N/A' }} | CR: {{ combatant.cr ?? 'N/A' }}
            <!-- '?? N/A' zeigt 'N/A' an, falls AC oder CR nicht definiert (null oder undefined) sind -->
          </div>
          <!-- Platzhalter für Status-Icons -->
          <div class="status-icons mt-1">
             <v-icon v-for="effect in combatant.statusEffects" :key="effect" small :title="effect" class="mr-1">
               mdi-star-circle-outline 
             </v-icon>
             <!-- TODO: Ersetze mdi-star-circle-outline durch passende Icons für Effekte -->
          </div>
        </v-col>

        <!-- Spalte 3: HP Anzeige -->
        <v-col cols="4" class="text-right">
          <div class="font-weight-medium">
            {{ combatant.currentHp }} / {{ combatant.maxHp }} HP
          </div>
          <!-- v-progress-linear: Zeigt die HP als Balken an -->
          <v-progress-linear
            :model-value="hpPercentage"
            :color="hpColor"
            height="8"
            rounded
            class="mt-1"
          ></v-progress-linear>
        </v-col>
      </v-row>
    </v-card-text>
  </v-card>
</template>

<style scoped>
.v-card {
  cursor: pointer; /* Zeigt an, dass man darauf klicken kann */
  transition: background-color 0.2s ease-in-out;
}
.initiative-input {
  max-width: 60px; /* Begrenzt die Breite des Initiative-Felds */
}
.status-icons .v-icon {
  font-size: 1rem; /* Kleinere Icons */
}
</style>