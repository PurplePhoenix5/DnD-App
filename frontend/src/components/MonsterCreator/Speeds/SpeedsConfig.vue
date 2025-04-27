<!-- frontend/src/components/MonsterCreator/Speeds/SpeedsConfig.vue -->
<script setup>
import { ref, onMounted, computed, nextTick } from 'vue';
import { loadDnDData } from '../../../utils/dndDataService.js'; // Pfad prüfen!
import { cloneDeep } from 'lodash'; // Für Array-Updates

const props = defineProps({
  modelValue: { type: Array, required: true }, // Erhält das 'speeds'-Array
  isEnabled: { type: Boolean, default: false } // Wird vom Parent übergeben
});
const emit = defineEmits(['update:field']); // Sendet spezifische Feld-Updates

// --- Lade Daten für Select ---
const movementTypes = ref([]);
const isLoadingData = ref(true);

onMounted(async () => {
    movementTypes.value = await loadDnDData('movementTypes.json') || [];
    isLoadingData.value = false;

    // Stelle sicher, dass mindestens eine Speed vorhanden ist, wenn die Komponente geladen wird
    if (props.modelValue.length === 0) {
        addNewSpeedLine();
    }
});

// --- Methoden zum Bearbeiten der Speeds ---

// Hilfsfunktion, um ein Update des gesamten Arrays zu emitten
function emitUpdate(newSpeedsArray) {
    emit('update:field', { key: 'speeds', value: newSpeedsArray });
}

// Aktualisiert ein spezifisches Feld einer Speed-Zeile
function updateSpeedField(index, field, value) {
    const newSpeeds = cloneDeep(props.modelValue); // Kopie erstellen
    if (newSpeeds[index]) {
        if (field === 'speed') {
            // Stelle sicher, dass Speed eine Zahl und >= 0 ist
            newSpeeds[index][field] = Math.max(0, parseInt(value, 10) || 0);
        } else {
            newSpeeds[index][field] = value;
        }
        emitUpdate(newSpeeds);
    }
}

// Fügt eine neue, leere Speed-Zeile hinzu
function addNewSpeedLine() {
    const newSpeeds = cloneDeep(props.modelValue);
    newSpeeds.push({
        speed: null, // Startet leer
        type: null,  // Startet leer
        note: ''
        // Keine ID nötig im Frontend, Backend kann sie ggf. generieren
    });
    emitUpdate(newSpeeds);
}

// Entfernt eine Speed-Zeile am gegebenen Index
function removeSpeedLine(index) {
    // Verhindere das Löschen der letzten Zeile
    if (props.modelValue.length <= 1) {
        alert("At least one speed must remain.");
        return;
    }
    const newSpeeds = cloneDeep(props.modelValue);
    newSpeeds.splice(index, 1); // Entferne das Element am Index
    emitUpdate(newSpeeds);
}

</script>

<template>
  <div>
    <div v-if="isLoadingData">Loading movement types...</div>
    <div v-else>
      <!-- Iteriere über das modelValue (das speeds-Array) -->
      <v-row v-for="(speed, index) in modelValue" :key="index" dense align="center" class="mb-2">

        <!-- Speed Input -->
        <v-col cols="12" md="2">
          <v-number-input
            label="Speed"
            :model-value="speed.speed"
            @update:model-value="updateSpeedField(index, 'speed', $event)"
            density="compact" variant="outlined"
            :min="0"
            :step="5" 
            control-variant="stacked" :reverse="false" inset
            suffix="ft." 
            :disabled="!isEnabled"
            hide-details="auto"
            class="left-aligned-input speed-input"
          />
        </v-col>

        <!-- Movement Type Select -->
        <v-col cols="12" md="4">
          <v-select
            label="Movement Type"
            :items="movementTypes"
            :model-value="speed.type"
            @update:model-value="updateSpeedField(index, 'type', $event)"
            density="compact" variant="outlined"
            :disabled="!isEnabled"
            hide-details="auto"
          />
        </v-col>

        <!-- Notes Input -->
        <v-col cols="12" md="5">
           <v-text-field
             label="Notes"
             :model-value="speed.note"
             @update:model-value="updateSpeedField(index, 'note', $event)"
             density="compact" variant="outlined"
             clearable
             :disabled="!isEnabled"
             hide-details="auto"
            />
        </v-col>

        <!-- Delete Button -->
        <v-col cols="12" md="1" class="text-right">
           <v-btn
              v-if="modelValue.length > 1" 
              icon="mdi-delete-outline"
              variant="text" color="error"
              size="small"
              @click="removeSpeedLine(index)"
              :disabled="!isEnabled"
              aria-label="Remove speed"
           />
           <!-- Button nur anzeigen, wenn mehr als eine Zeile da ist -->
        </v-col>
      </v-row>

      <!-- Add Speed Button -->
      <v-row dense>
          <v-col cols="12">
              <v-btn
                @click="addNewSpeedLine"
                block variant="outlined"
                prepend-icon="mdi-plus"
                :disabled="!isEnabled"
                class="mt-2"
              >
                Add Speed
              </v-btn>
          </v-col>
      </v-row>
    </div>
  </div>
</template>

<style scoped>
/* Styles für linksbündigen Input (ggf. anpassen) */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
  /* Passe Padding für Suffix und Buttons an */
   padding-right: 5px !important; /* Beispielwert */
}
:deep(.left-aligned-input .v-field__append-inner) {
    align-items: center;
    padding-inline-start: 0 !important;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important;
    padding: 0 !important;
    height: 50% !important;
    font-size: 0.8em;
}
/* Passe Padding Rechts an, um Platz für Suffix UND Buttons zu machen */
:deep(.speed-input .v-field__input) {
  /* Mehr Platz rechts nötig: Suffix (~20px?) + Buttons (~25px?) + etwas Luft */
  padding-right: 5px !important; /* Beispielwert, anpassen! */
}

/* Style für Suffix im Speed-Input */
:deep(.speed-input .v-field__suffix) {
    padding-inline-start: 2px !important; /* Reduziere linken Abstand des Suffix stark */
    padding-inline-end: 2px !important; /* Ggf. auch rechten Abstand anpassen */
    opacity: 0.8; /* Suffix etwas dezenter */
    /* Positioniere Suffix relativ zum append-inner Slot */
    position: relative;
    right: 25px; /* Verschiebe es nach links (Wert entspricht ungefähr Button-Breite), anpassen! */
}

/* Stelle sicher, dass append-inner (Buttons) genügend Platz hat */
:deep(.speed-input .v-field__append-inner) {
    padding-left: 4px; /* Kleiner Abstand nach den Buttons */
}
</style>