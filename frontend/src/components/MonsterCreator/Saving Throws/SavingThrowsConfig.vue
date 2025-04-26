<!-- frontend/src/components/MonsterCreator/SavingThrows/SavesConfig.vue -->
<script setup>
import { computed, ref, watch } from 'vue';
// Kein lodash nötig, da wir spezifische Pfade emitten
import { saveModifierForStat, renderBonus } from '../../../utils/mathRendering.js'; // Pfad prüfen

const props = defineProps({
  modelValue: { type: Object, required: true }, // Erhält das 'saves'-Objekt
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
        proficient: computed({
            get: () => props.modelValue[statKey]?.proficient ?? false,
            set: (v) => updateSaveValue(statKey, 'proficient', v)
        }),
        overrideValue: computed({
            get: () => props.modelValue[statKey]?.overrideValue,
            set: (v) => {
                 const newOverride = v === '' || v === null ? null : parseInt(v, 10);
                 updateSaveValue(statKey, 'overrideValue', newOverride);
                 overrideStates.value[statKey] = newOverride !== null;
            }
        }),
        // === Geändert: Default Value liest nur noch aus Prop ===
        // Die Berechnung findet jetzt ausschließlich im Parent statt
        defaultValue: computed(() => props.modelValue[statKey]?.defaultValue ?? 0),
        // ======================================================
        displayValue: computed(() => {
             return overrideStates.value[statKey]
                 ? props.modelValue[statKey]?.overrideValue // Zeige Override
                 : props.modelValue[statKey]?.defaultValue; // Zeige Default aus Prop
        }),
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

function toggleProficiency(statKey) {
    const currentProficiency = props.modelValue[statKey]?.proficient ?? false;
    updateSaveValue(statKey, 'proficient', !currentProficiency);
    // Wichtig: Neuberechnung des Defaults wird im Parent getriggert
}
// ----------------------------

</script>

<template>
  <div>
    <v-row dense>
      <!-- Iteriere über die Stats für die Felder -->
      <v-col cols="12" md="4" v-for="statKey in ['STR', 'DEX', 'CON', 'INT', 'WIS', 'CHA']" :key="statKey">
        <v-number-input
          :label="`${statKey} Save`"
          :model-value="savesComputed[statKey].displayValue.value"
          @update:model-value="savesComputed[statKey].overrideValue.value = $event === '' || $event === null ? null : parseInt($event, 10)"
          density="compact" variant="outlined"
          :readonly="!savesComputed[statKey].isOverridden.value"
          :class="{'input-is-default': !savesComputed[statKey].isOverridden.value}"
          control-variant="stacked" :reverse="false" inset
          class="left-aligned-input"
          :disabled="!isEnabled"
        >
          <!-- Prepend-Inner Slot für Proficiency Toggle -->
          <template v-slot:prepend-inner>
            <v-tooltip location="top" :text="modelValue[statKey]?.proficient ? 'Proficient' : 'Not Proficient'">
                <template v-slot:activator="{ props: tooltipProps }">
                    <v-btn
                        v-bind="tooltipProps"
                        :icon="modelValue[statKey]?.proficient ? 'mdi-check-circle' : 'mdi-circle-outline'"
                        :color="modelValue[statKey]?.proficient ? 'success' : 'grey'"
                        variant="text" size="x-small"
                        @click.stop="toggleProficiency(statKey)"
                        :disabled="!isEnabled || savesComputed[statKey].isOverridden.value" 
                        class="mr-1"
                    />
                    <!-- Sperre Prof-Toggle, wenn Override aktiv ist -->
                </template>
            </v-tooltip>
          </template>

          <!-- Append-Inner Slot für Override Toggle -->
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