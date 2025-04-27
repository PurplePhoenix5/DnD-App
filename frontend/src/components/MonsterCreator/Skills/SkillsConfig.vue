<!-- frontend/src/components/MonsterCreator/Skills/SkillsConfig.vue -->
<script setup>
import { computed, ref, onMounted, watch } from 'vue';
import { cloneDeep, set } from 'lodash';
import { loadDnDData } from '../../../utils/dndDataService.js';
import { calculateSkillBonus, renderBonus } from '../../../utils/mathRendering.js'; // Pfad prüfen

const props = defineProps({
  modelValue: { type: Array, required: true }, // Erhält das 'skills'-Array
  basicsData: { type: Object, required: true }, // Brauchen Stats und PB
  isEnabled: { type: Boolean, default: false }
});
const emit = defineEmits(['update:field']);

// --- Refs für geladene Daten ---
const allSkillsData = ref({}); // Objekt { SkillName: { stat: 'XYZ' } }
const allSkillNames = ref([]); // Array der Skill-Namen für das Select
const isLoadingData = ref(true);

// --- Lokale Refs für Override Toggles (pro Skill) ---
// Da sich das Array ändern kann, verwenden wir ein Objekt, das über den Skill-Namen indiziert wird
const overrideStates = ref({});
// Initialisiere overrideStates basierend auf dem aktuellen modelValue
function initializeOverrideStates(skillsArray) {
    const newStates = {};
    (skillsArray || []).forEach(skill => {
        newStates[skill.skill] = skill.overrideValue !== null && skill.overrideValue !== undefined;
    });
    overrideStates.value = newStates;
}
watch(() => props.modelValue, (newSkills) => {
    initializeOverrideStates(newSkills);
}, { deep: true, immediate: true });
// -----------------------------------------

onMounted(async () => {
    const skillsJson = await loadDnDData('skills.json');
    if (skillsJson) {
        allSkillsData.value = skillsJson;
        // Erstelle Liste aller Skill-Namen, sortiert, ohne Initiative
        allSkillNames.value = Object.keys(skillsJson)
            .filter(key => key !== 'Initiative')
            .sort();
    }
    isLoadingData.value = false;
});

// Hilfsfunktion zum Emitten von Updates (aktualisiert das GESAMTE Array)
function emitUpdate(newSkillsArray) {
    // Filtere leere oder ungültige Einträge ggf. hier raus
    emit('update:field', { key: 'skills', value: newSkillsArray });
}

// Aktualisiert ein Feld für einen bestimmten Skill im Array
function updateSkillField(index, field, value) {
    const newSkills = cloneDeep(props.modelValue);
    if (newSkills[index]) {
        newSkills[index][field] = value;
        // Wenn Override gesetzt wird, aktualisiere den lokalen Toggle-State
        if (field === 'overrideValue') {
             overrideStates.value[newSkills[index].skill] = value !== null && value !== undefined;
        }
        emitUpdate(newSkills);
    }
}

// Fügt einen neuen Skill hinzu
function addSkill(skillName) {
    if (!skillName || props.modelValue.some(s => s.skill === skillName)) {
        // Skill schon vorhanden oder kein Name ausgewählt
        return;
    }
    const newSkills = cloneDeep(props.modelValue);
    // Füge den neuen Skill mit Default-Werten hinzu
    const newSkillEntry = {
        skill: skillName,
        proficient: false,
        expertise: false,
        defaultValue: calculateSkillBonus(props.basicsData, { skill: skillName, proficient: false, expertise: false }, allSkillsData.value), // Berechne initialen Default
        overrideValue: null
    };
    newSkills.push(newSkillEntry);
    // Initialisiere Override State für den neuen Skill
    overrideStates.value[skillName] = false;
    emitUpdate(newSkills);
}

// Entfernt einen Skill
function removeSkill(index) {
    const newSkills = cloneDeep(props.modelValue);
    const removedSkill = newSkills.splice(index, 1);
    if (removedSkill && removedSkill[0]) {
        delete overrideStates.value[removedSkill[0].skill]; // Entferne aus lokalen States
    }
    emitUpdate(newSkills);
}

// --- Computed Property für verfügbare Skills zum Hinzufügen ---
// Filtert Skills heraus, die bereits im modelValue vorhanden sind
const availableSkillsToAdd = computed(() => {
    const addedSkillNames = new Set(props.modelValue.map(s => s.skill));
    return allSkillNames.value.filter(name => !addedSkillNames.has(name));
});

// --- Logik für Proficiency/Expertise Toggle Button ---
function cycleSkillProficiency(index) {
    const skill = props.modelValue[index];
    if (!skill) return;
    let newProf = false;
    let newExp = false;

    if (!skill.proficient && !skill.expertise) { // War 'none'
        newProf = true;
    } else if (skill.proficient && !skill.expertise) { // War 'proficient'
        newProf = true;
        newExp = true;
    } // else: war 'expertise', wird wieder 'none'

    // Update direkt beide Felder
    const newSkills = cloneDeep(props.modelValue);
    newSkills[index].proficient = newProf;
    newSkills[index].expertise = newExp;
     // Berechne neuen Default-Wert neu und speichere ihn
     newSkills[index].defaultValue = calculateSkillBonus(props.basicsData, newSkills[index], allSkillsData.value);
    emitUpdate(newSkills);
}

function getSkillProfIcon(index) {
    const skill = props.modelValue[index];
    if (skill?.expertise) return 'mdi-chevron-double-up';
    if (skill?.proficient) return 'mdi-chevron-up';
    return 'mdi-minus';
}
function getSkillProfTooltip(index) {
    const skill = props.modelValue[index];
    if (skill?.expertise) return 'Expertise';
    if (skill?.proficient) return 'Proficiency';
    return 'Not Proficient';
}
// -------------------------------------------------------

// --- Logik für Override Toggle Button ---
function toggleSkillOverride(index) {
    const skill = props.modelValue[index];
    if (!skill) return;
    const statKey = skill.skill; // Verwende Skill-Namen als Key
    const nextState = !overrideStates.value[statKey];
    overrideStates.value[statKey] = nextState;

    // Update den overrideValue im Parent
    updateSkillField(index, 'overrideValue', nextState ? (skill.defaultValue ?? 0) : null);
}
// -----------------------------------

// --- Watcher zum Neuberechnen der Defaults ---
watch([() => props.basicsData.stats, () => props.basicsData.PB, allSkillsData],
    () => {
         if (!props.modelValue || !props.basicsData || Object.keys(allSkillsData.value).length === 0) return; // Warte auf Daten

         console.log("SkillsConfig: Recalculating skill defaults...");
         const newSkills = cloneDeep(props.modelValue);
         let changed = false;
         newSkills.forEach(skill => {
              // Berechne neuen Default nur, wenn kein Override aktiv ist
              if (skill.overrideValue === null || skill.overrideValue === undefined) {
                   const newDefault = calculateSkillBonus(props.basicsData, skill, allSkillsData.value);
                   if (skill.defaultValue !== newDefault) {
                       skill.defaultValue = newDefault;
                       changed = true;
                   }
              }
         });
         // Sende nur ein Update, wenn sich tatsächlich Defaults geändert haben
         if (changed) {
            console.log("SkillsConfig: Updating defaults in parent.");
            emitUpdate(newSkills);
         }
    },
    { deep: true } // Kein immediate, da wir auf geladene Daten warten
);
// -----------------------------------------

</script>

<template>
  <div>
    <div v-if="isLoadingData">Loading skills data...</div>
    <div v-else>
      <v-row dense>
        <!-- Iteriere über die Skills im modelValue und zeige je 2 pro Zeile -->
        <v-col cols="12" md="6" v-for="(skill, index) in modelValue" :key="skill.skill">
          <div class="d-flex align-center">
             <!-- Number Input für den Skill Bonus -->
             <v-number-input
               :label="skill.skill"
               :model-value="overrideStates[skill.skill] ? skill.overrideValue : skill.defaultValue"
               @update:model-value="updateSkillField(index, 'overrideValue', $event === '' || $event === null ? null : parseInt($event, 10))"
               density="compact" variant="outlined"
               :readonly="!overrideStates[skill.skill]"
               :class="{'input-is-default': !overrideStates[skill.skill]}"
               control-variant="stacked" :reverse="false" inset
               class="left-aligned-input flex-grow-1"
               :disabled="!isEnabled"
             >
               <!-- Prepend-Inner Slot für Proficiency/Expertise Toggle -->
               <template v-slot:prepend-inner>
                  <v-tooltip location="top" :text="getSkillProfTooltip(index)">
                     <template v-slot:activator="{ props: tooltipProps }">
                         <v-btn
                             v-bind="tooltipProps"
                             :icon="getSkillProfIcon(index)"
                             :color="skill.proficient ? (skill.expertise ? 'primary' : 'success') : 'grey'"
                             variant="text" size="x-small"
                             @click.stop="cycleSkillProficiency(index)"
                             :disabled="!isEnabled || overrideStates[skill.skill]"
                             class="mr-1"
                         />
                     </template>
                  </v-tooltip>
               </template>

               <!-- Append-Inner Slot für Override Toggle -->
               <template v-slot:append-inner>
                  <v-tooltip location="top" :text="overrideStates[skill.skill] ? 'Use Default' : 'Override Bonus'">
                      <template v-slot:activator="{ props: tooltipProps }">
                          <v-btn
                              v-bind="tooltipProps"
                              :icon="overrideStates[skill.skill] ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                              variant="text" size="x-small"
                              @click.stop="toggleSkillOverride(index)"
                              :disabled="!isEnabled"
                          />
                      </template>
                  </v-tooltip>
               </template>
             </v-number-input>

             <!-- Delete Button daneben -->
             <v-btn
                icon="mdi-delete-outline"
                variant="text" color="error"
                size="x-small" class="ml-1"
                @click="removeSkill(index)"
                :disabled="!isEnabled"
                aria-label="Remove skill"
             />
          </div>
        </v-col>
      </v-row>

      <!-- Add Skill Button/Select -->
      <v-row dense>
          <v-col cols="12">
              <v-select
                label="Add Skill"
                :items="availableSkillsToAdd"
                @update:model-value="addSkill($event); $event = null" 
                density="compact" variant="outlined"
                :disabled="!isEnabled || availableSkillsToAdd.length === 0"
                hide-details
                clearable
                class="mt-2"
                no-data-text="All skills added"
              >
                <template v-slot:prepend-item>
                  <v-list-item>
                    <v-list-item-title class="text-disabled text-caption">Select a skill to add</v-list-item-title>
                  </v-list-item>
                   <v-divider></v-divider>
                </template>
              </v-select>
          </v-col>
      </v-row>
    </div>
  </div>
</template>

<style scoped>
/* Styles für Input-Ausrichtung etc. */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
  padding-right: 70px !important; /* Platz für 2 Buttons */
  padding-left: 30px !important; /* Platz für 1 Button */
}
:deep(.left-aligned-input .v-field__prepend-inner),
:deep(.left-aligned-input .v-field__append-inner) {
    align-items: center;
    padding-top: 0 !important;
    padding-bottom: 0 !important;
}
:deep(.left-aligned-input .v-field__prepend-inner .v-btn),
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important;
    padding: 0 2px !important;
    margin: 0 1px !important;
}
.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
}
</style>