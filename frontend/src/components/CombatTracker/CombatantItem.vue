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
const localInitiative = ref(props.combatant.initiative);

// --- Konstanten für Min/Max ---
const MIN_INITIATIVE = 0;
const MAX_INITIATIVE = 30;

// --- Computed Properties ---
const hpPercentage = computed(() => {
  if (!props.combatant.maxHp || props.combatant.maxHp <= 0) return 0;
  return (props.combatant.currentHp / props.combatant.maxHp) * 100;
});

const hpColor = computed(() => {
  if (hpPercentage.value > 50) return 'success';
  if (hpPercentage.value > 25) return 'warning';
  return 'error';
});

function getConditionIconPath(conditionId) {
  // Annahme: Bilder liegen in public/assets/conditions/ und haben den Namen der ID.png
  return `/assets/conditions/${conditionId}.png`;
}

// --- Methoden ---
function selectThisCombatant() {
  emit('select-combatant', props.combatant.id);
}

function submitInitiativeChange() {
  // 1. Parse den Wert
  let newInitiative = parseInt(localInitiative.value, 10);

  // 2. Prüfe, ob es eine gültige Zahl ist
  if (!isNaN(newInitiative)) {
    // 3. Wertebereich erzwingen (Clamping)
    newInitiative = Math.max(MIN_INITIATIVE, Math.min(newInitiative, MAX_INITIATIVE));

    // 4. Aktualisiere das lokale Feld auf den ggf. geklemmten Wert
    //    (Wichtig, falls der Benutzer z.B. 50 eingegeben hat)
    localInitiative.value = newInitiative;

    // 5. Nur senden, wenn sich der (ggf. geklemmte) Wert vom Original unterscheidet
    if (newInitiative !== props.combatant.initiative) {
      emit('update-initiative', {
        combatantId: props.combatant.id,
        newInitiative: newInitiative
      });
    }
  } else {
    // Setze das lokale Feld zurück auf den Prop-Wert, falls die Eingabe ungültig war
    localInitiative.value = props.combatant.initiative;
  }
}

// --- Watcher ---
// Wenn sich die Initiative von außen ändert, aktualisiere das lokale Textfeld.
watch(() => props.combatant.initiative, (newVal) => {
  // Stelle sicher, dass auch der initiale Wert im erlaubten Bereich liegt (optional, aber sicher)
  localInitiative.value = Math.max(MIN_INITIATIVE, Math.min(newVal, MAX_INITIATIVE));
}, { immediate: true }); // immediate: true führt dies auch beim ersten Laden aus

</script>

<template>
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
          <v-text-field
            v-model.number="localInitiative"
            type="number"
            :min="MIN_INITIATIVE"  
            :max="MAX_INITIATIVE"  
            density="compact"
            hide-details
            variant="outlined"
            @blur="submitInitiativeChange"
            @keydown.enter="submitInitiativeChange"
            @click.stop
            aria-label="Initiative"
            class="initiative-input" 
          />
        </v-col>

        <!-- Spalte 2: Name, AC, CR -->
        <v-col cols="6" class="pl-3"> <!-- Kleines Padding links erhöht -->
          <div class="font-weight-bold">{{ combatant.name }}</div>
          <div class="text-caption">
            AC: {{ combatant.ac ?? 'N/A' }} | CR: {{ combatant.cr ?? 'N/A' }}
          </div>
          <div class="status-icons mt-1 d-flex ga-1"> <!-- d-flex für nebeneinander, ga-1 für Lücke -->
            <!-- Iteriere über die statusEffects des *aktuellen* combatant -->
            <div v-for="effectId in combatant.statusEffects" :key="effectId">
              <!-- Tooltip für den Namen beim Hovern -->
              <v-tooltip location="top">
                <template v-slot:activator="{ props: tooltipProps }">
                  <!-- Das Icon selbst -->
                  <v-img
                    v-bind="tooltipProps" 
                    :src="getConditionIconPath(effectId)"
                    :alt="effectId" 
                    width="20"  
                    height="20" 
                    contain
                    class="active-condition-icon"
                  ></v-img>
                </template>
                 <!-- Text im Tooltip (optional, formatiere den Namen schöner) -->
                <span>{{ effectId.charAt(0).toUpperCase() + effectId.slice(1) }}</span>
              </v-tooltip>
            </div>
          </div>
        </v-col>

        <!-- Spalte 3: HP Anzeige -->
        <v-col cols="4" class="text-right">
          <div class="font-weight-medium">
            {{ combatant.currentHp }} / {{ combatant.maxHp }} HP
          </div>
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
  cursor: pointer;
  transition: background-color 0.2s ease-in-out;
}

.initiative-input {
  /* Erhöhe die maximale Breite, um Platz für Zahl und Spinner zu schaffen */
  max-width: 85px; /* Probier diesen Wert aus, passe ihn ggf. an */
}

/* Optional: Verhindere, dass der Browser die Standard-Spinner für Input[type=number] anzeigt, */
/* falls Vuetify eigene hinzufügt oder du sie ganz weghaben willst (nicht empfohlen für type=number) */
/*
.initiative-input input[type='number']::-webkit-outer-spin-button,
.initiative-input input[type='number']::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}
.initiative-input input[type='number'] {
  -moz-appearance: textfield;
}
*/

.status-icons .v-icon {
  font-size: 1rem;
}
</style>