<!-- frontend/src/components/MonsterCreator/Senses/SensesConfig.vue -->
<script setup>
import { computed, ref, watch } from 'vue';
import { cloneDeep, set, get } from 'lodash';
import { calculateSkillBonus, statModifier, renderBonus } from '../../../utils/mathRendering.js'; // Pfad prüfen!

const props = defineProps({
  modelValue: { type: Object, required: true }, // Erhält das 'senses'-Objekt
  basicsData: { type: Object, required: true }, // Für Stats/PB
  skillsData: { type: Array, required: true }, // Das Array monsterBeingCreated.skills
  allSkillsInfo: { type: Object, required: true }, // Geladene skills.json Daten
  isEnabled: { type: Boolean, default: false }
});
const emit = defineEmits(['update:field']);

// --- Lokale Refs für Override Toggles ---
const isPPOverride = ref(props.modelValue.passivePerception?.overrideValue !== null);
const isPIOverride = ref(props.modelValue.passiveInsight?.overrideValue !== null);

watch(() => props.modelValue.passivePerception?.overrideValue, (newVal) => { isPPOverride.value = newVal !== null; });
watch(() => props.modelValue.passiveInsight?.overrideValue, (newVal) => { isPIOverride.value = newVal !== null; });
// -----------------------------------------

// Hilfsfunktion zum Emitieren
function updateSenseValue(key, value) {
    emit('update:field', { key: `senses.${key}`, value: value });
}

// --- Computed Properties für v-model ---
const blindsight = computed({
    get: () => props.modelValue.blindsight ?? 0,
    set: (v) => updateSenseValue('blindsight', Math.max(0, parseInt(v, 10) || 0))
});
const darkvision = computed({
    get: () => props.modelValue.darkvision ?? 0,
    set: (v) => updateSenseValue('darkvision', Math.max(0, parseInt(v, 10) || 0))
});
const tremorsense = computed({
    get: () => props.modelValue.tremorsense ?? 0,
    set: (v) => updateSenseValue('tremorsense', Math.max(0, parseInt(v, 10) || 0))
});
const truesight = computed({
    get: () => props.modelValue.truesight ?? 0,
    set: (v) => updateSenseValue('truesight', Math.max(0, parseInt(v, 10) || 0))
});
const sensesNotes = computed({
    get: () => props.modelValue.sensesNotes ?? '',
    set: (v) => updateSenseValue('sensesNotes', v || '')
});

// --- Passive Perception ---
const ppOverrideValue = computed({
    get: () => props.modelValue.passivePerception?.overrideValue,
    set: (v) => {
        const newValue = v === '' || v === null ? null : parseInt(v, 10);
        updateSenseValue('passivePerception.overrideValue', newValue);
        isPPOverride.value = newValue !== null;
    }
});
const ppDefaultValue = computed(() => props.modelValue.passivePerception?.defaultValue ?? 10);
const displayedPP = computed(() => isPPOverride.value ? ppOverrideValue.value : ppDefaultValue.value);

// --- Passive Insight ---
const piOverrideValue = computed({
    get: () => props.modelValue.passiveInsight?.overrideValue,
    set: (v) => {
        const newValue = v === '' || v === null ? null : parseInt(v, 10);
        updateSenseValue('passiveInsight.overrideValue', newValue);
        isPIOverride.value = newValue !== null;
    }
});
const piDefaultValue = computed(() => props.modelValue.passiveInsight?.defaultValue ?? 10);
const displayedPI = computed(() => isPIOverride.value ? piOverrideValue.value : piDefaultValue.value);

// --- Toggle Methoden ---
function togglePPOverride() {
    const nextState = !isPPOverride.value;
    isPPOverride.value = nextState;
    updateSenseValue('passivePerception.overrideValue', nextState ? (ppDefaultValue.value ?? 10) : null);
}
function togglePIOverride() {
    const nextState = !isPIOverride.value;
    isPIOverride.value = nextState;
    updateSenseValue('passiveInsight.overrideValue', nextState ? (piDefaultValue.value ?? 10) : null);
}

</script>

<template>
  <div>
    <v-row dense>
      <!-- Zeile 1 -->
      <v-col cols="12" md="2">
        <v-number-input label="Darkvision" v-model="darkvision" :min="0" :step="5" suffix="ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset class="left-aligned-input" :disabled="!isEnabled" />
      </v-col>
      <v-col cols="12" md="2">
        <v-number-input label="Blindsight" v-model="blindsight" :min="0" :step="5" suffix="ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset class="left-aligned-input" :disabled="!isEnabled" />
      </v-col>
      <v-col cols="12" md="2">
        <v-number-input label="Tremorsense" v-model="tremorsense" :min="0" :step="5" suffix="ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset class="left-aligned-input" :disabled="!isEnabled" />
      </v-col>
      <v-col cols="12" md="2">
        <v-number-input label="Truesight" v-model="truesight" :min="0" :step="5" suffix="ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset class="left-aligned-input" :disabled="!isEnabled" />
      </v-col>
      <v-col cols="12" md="2">
        <!-- Passive Perception -->
        <v-number-input
            label="Pass. Perc."
            :model-value="displayedPP"
            @update:model-value="ppOverrideValue = $event"
            density="compact" variant="outlined"
            :readonly="!isPPOverride"
            :class="{'input-is-default': !isPPOverride}"
            :hint="isPPOverride ? 'Manual override' : `Default: ${ppDefaultValue}`"
            persistent-hint clearable
            control-variant="stacked" :reverse="false" inset
            class="left-aligned-input" :disabled="!isEnabled"
        >
           <template v-slot:append-inner>
               <v-tooltip location="top" :text="isPPOverride ? 'Use Default' : 'Override'">
                   <template v-slot:activator="{ props: tooltipProps }">
                       <v-btn v-bind="tooltipProps" :icon="isPPOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'" variant="text" size="x-small" @click.stop="togglePPOverride" :disabled="!isEnabled"/>
                   </template>
               </v-tooltip>
           </template>
        </v-number-input>
      </v-col>
      <v-col cols="12" md="2">
         <!-- Passive Insight -->
        <v-number-input
            label="Pass. Insight"
            :model-value="displayedPI"
            @update:model-value="piOverrideValue = $event"
            density="compact" variant="outlined"
            :readonly="!isPIOverride"
            :class="{'input-is-default': !isPIOverride}"
            :hint="isPIOverride ? 'Manual override' : `Default: ${piDefaultValue}`"
            persistent-hint clearable
            control-variant="stacked" :reverse="false" inset
            class="left-aligned-input" :disabled="!isEnabled"
        >
           <template v-slot:append-inner>
               <v-tooltip location="top" :text="isPIOverride ? 'Use Default' : 'Override'">
                   <template v-slot:activator="{ props: tooltipProps }">
                       <v-btn v-bind="tooltipProps" :icon="isPIOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'" variant="text" size="x-small" @click.stop="togglePIOverride" :disabled="!isEnabled"/>
                   </template>
               </v-tooltip>
           </template>
        </v-number-input>
      </v-col>

       <!-- Zeile 2 -->
        <v-col cols="12">
            <v-text-field label="Senses Notes" v-model="sensesNotes" density="compact" variant="outlined" clearable :disabled="!isEnabled"/>
        </v-col>
    </v-row>
  </div>
</template>

<style scoped>
/* Styles für linksbündigen Input und Suffix */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
  padding-right: 60px !important; /* Platz für Suffix UND Buttons */
}
:deep(.left-aligned-input .v-field__append-inner) {
    align-items: center;
    padding-inline-start: 0 !important;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important; padding: 0 4px !important; margin: 0 1px !important;
}
:deep(.v-number-input .v-field__suffix) {
    padding-inline-start: 2px !important; padding-inline-end: 2px !important; opacity: 0.8;
    /* Positioniere Suffix relativ zum append-inner Slot (Buttons) */
    position: relative; right: 30px; /* Wert an Button-Breite anpassen */
}
.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
}
</style>