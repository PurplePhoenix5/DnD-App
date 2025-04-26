<!-- frontend/src/components/MonsterCreator/SavingThrows/SavesConfig.vue -->
<script setup>
import { computed, ref, watch } from 'vue';
// Kein lodash nötig, da wir spezifische Pfade emitten
import { saveModifierForStat, renderBonus } from '../../../utils/mathRendering.js'; // Pfad prüfen

const props = defineProps({
  modelValue: { type: Object, required: true }, // Erhält das 'saves'-Objekt
  // Empfange das Basics-Objekt, um auf Stats und PB zugreifen zu können
  basicsData: { type: Object, required: true },
  isEnabled: { type: Boolean, default: false } // Wird vom Parent übergeben
});
const emit = defineEmits(['update:field']); // Sendet spezifische Feld-Updates

// --- Lokale Refs für Override Toggles (pro Stat) ---
const overrideStates = ref({
    STR: props.modelValue.STR?.overrideValue !== null,
    DEX: props.modelValue.DEX?.overrideValue !== null,
    CON: props.modelValue.CON?.overrideValue !== null,
    INT: props.modelValue.INT?.overrideValue !== null,
    WIS: props.modelValue.WIS?.overrideValue !== null,
    CHA: props.modelValue.CHA?.overrideValue !== null,
});

// Watcher, um lokale Toggles bei externer Änderung zu aktualisieren
watch(() => props.modelValue, (newSaves) => {
    for (const statKey in overrideStates.value) {
         overrideStates.value[statKey] = newSaves[statKey]?.overrideValue !== null;
    }
}, { deep: true });
// ---------------------------------------------

// Hilfsfunktion zum Emitten von Updates
function updateSaveValue(statKey, field, value) {
    // Pfad ist z.B. 'saves.STR.proficient'
    emit('update:field', { key: `saves.${statKey}.${field}`, value: value });
}

// --- Computed Properties für jeden Save ---
function createSaveComputed(statKey) {
    return {
        // Proficient Checkbox
        proficient: computed({
            get: () => props.modelValue[statKey]?.proficient ?? false,
            set: (v) => updateSaveValue(statKey, 'proficient', v)
        }),
        // Override Input
        overrideValue: computed({
            get: () => props.modelValue[statKey]?.overrideValue,
            set: (v) => {
                 const newOverride = v === '' || v === null ? null : parseInt(v, 10);
                 updateSaveValue(statKey, 'overrideValue', newOverride);
                 // Update lokalen Toggle State
                 overrideStates.value[statKey] = newOverride !== null;
            }
        }),
        // Berechneter Default Wert (basierend auf Monster Stats/PB und Proficiency)
        defaultValue: computed(() => {
            // Erstelle temporäres Monster-Objekt für saveModifierForStat
            const tempMonster = {
                basics: props.basicsData,
                saves: { [statKey]: props.modelValue[statKey] } // Nur relevanter Save
            };
             // Berechne den Wert *ohne* Berücksichtigung des Overrides
             // saveModifierForStat berechnet den Bonus basierend auf Stat und Proficiency
             return saveModifierForStat(tempMonster, statKey);
        }),
        // Wert, der im Feld angezeigt wird
        displayValue: computed(() => {
             return overrideStates.value[statKey]
                 ? props.modelValue[statKey]?.overrideValue // Zeige Override
                 : props.modelValue[statKey]?.defaultValue; // Zeige Default
        }),
        // Ist Override für diesen Stat aktiv?
        isOverridden: computed(() => overrideStates.value[statKey])
    };
}

const savesComputed = {
    STR: createSaveComputed('STR'),
    DEX: createSaveComputed('DEX'),
    CON: createSaveComputed('CON'),
    INT: createSaveComputed('INT'),
    WIS: createSaveComputed('WIS'),
    CHA: createSaveComputed('CHA'),
};
// --------------------------------------

// --- Override Toggle Methoden ---
function toggleOverride(statKey) {
    const nextState = !overrideStates.value[statKey];
    overrideStates.value[statKey] = nextState;
    if (!nextState) {
        updateSaveValue(statKey, 'overrideValue', null); // Entferne Override
    }
    // Wenn aktiviert, kann Nutzer tippen
}
// ----------------------------

// --- Watcher zum Neuberechnen der Defaults bei Stat/PB/Prof Änderung ---
// Diese Watcher müssen im Parent (MonsterCreator) sein, da sie auf
// Änderungen in *anderen* Teilen des Monster-Objekts reagieren müssen.
// Hier berechnen wir den Default nur zur Anzeige neu.

watch([() => props.basicsData.stats, () => props.basicsData.PB, () => props.modelValue],
    () => {
         // Berechne alle Default-Werte neu und update sie im State, wenn KEIN Override aktiv ist
         for (const statKey in props.modelValue) {
              if (overrideStates.value[statKey] === false) { // Nur wenn kein Override
                  const tempMonster = { basics: props.basicsData, saves: { [statKey]: props.modelValue[statKey] } };
                  const newDefault = saveModifierForStat(tempMonster, statKey);
                  // Update den Default-Wert im Parent über das Event
                   if (props.modelValue[statKey]?.defaultValue !== newDefault) {
                       updateSaveValue(statKey, 'defaultValue', newDefault);
                   }
              }
         }
    },
    { deep: true, immediate: true } // Immediate, um initiale Defaults zu berechnen
);
// -------------------------------------------------------------

</script>

<template>
  <div>
    <v-row dense>
      <!-- Iteriere über die Stats für die Felder -->
      <v-col cols="12" md="4" v-for="statKey in ['STR', 'DEX', 'CON', 'INT', 'WIS', 'CHA']" :key="statKey">
        <div class="d-flex align-center">
          <!-- Checkbox für Proficiency -->
          <v-checkbox-btn
            v-model="savesComputed[statKey].proficient.value"
            :disabled="!isEnabled || savesComputed[statKey].isOverridden.value" 
            class="mr-1"
            density="compact"
          >
             <template v-slot:label>
                 <v-tooltip location="top" :text="`${statKey} Save Proficient`">
                     <template v-slot:activator="{ props: tooltipProps }">
                         <span v-bind="tooltipProps">{{ statKey }}</span>
                     </template>
                 </v-tooltip>
             </template>
          </v-checkbox-btn>

          <!-- Number Input für den Save Bonus -->
          <v-number-input
            :label="`${statKey} Save`"
            :model-value="savesComputed[statKey].displayValue.value" 
            @update:model-value="savesComputed[statKey].overrideValue.value = $event === '' || $event === null ? null : parseInt($event, 10)"
            density="compact" variant="outlined"
            :readonly="!savesComputed[statKey].isOverridden.value" 
            :class="{'input-is-default': !savesComputed[statKey].isOverridden.value}" 
            :hint="savesComputed[statKey].isOverridden.value ? 'Manual override' : 'Calculated default'"
            persistent-hint
            control-variant="stacked" :reverse="false" inset
            class="left-aligned-input flex-grow-1"
            :disabled="!isEnabled" 
          >
             <template v-slot:append-inner>
               <v-tooltip location="top" :text="savesComputed[statKey].isOverridden.value ? 'Use Default Save' : 'Override Save'">
                   <template v-slot:activator="{ props: tooltipProps }">
                       <v-btn
                           v-bind="tooltipProps"
                           :icon="savesComputed[statKey].isOverridden.value ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                           variant="text" size="x-small"
                           @click.stop="toggleOverride(statKey)"
                           :disabled="!isEnabled" 
                       />
                   </template>
               </v-tooltip>
             </template>
          </v-number-input>
        </div>
      </v-col>
    </v-row>
  </div>
</template>

<style scoped>
/* Styles wie in BasicsConfig */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
  padding-right: 40px !important; /* Platz für einen Button */
}
:deep(.left-aligned-input .v-field__append-inner) {
    align-items: center;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important;
    padding: 0 4px !important;
}
.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
  /* font-style: italic; */ /* Kursiv vielleicht nicht ideal für Zahlen */
}
.v-checkbox-btn {
    margin-right: -10px; /* Checkbox näher an den Input rücken */
    flex-shrink: 0;
}
.v-checkbox-btn .v-label {
    font-weight: bold;
}
</style>