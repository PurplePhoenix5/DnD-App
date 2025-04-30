<!-- frontend/src/components/MonsterCreator/ResistancesImmunities/ResistancesAndImmunities.vue -->
<script setup>
import { ref, onMounted, computed } from 'vue';
import { loadDnDData } from '../../../utils/dndDataService.js'; // Pfad prüfen!
import { cloneDeep, set } from 'lodash';

const props = defineProps({
  modelValue: { type: Object, required: true }, // Erhält { resistances: [], immunities: [], vulnerabilities: [], conditionImmunities: [] }
  isEnabled: { type: Boolean, default: false }
});
const emit = defineEmits(['update:field']); // Sendet spezifische Feld-Updates

// --- Lade Daten für Comboboxes ---
const damageOptions = ref([]);
const conditionOptions = ref([]);
const isLoadingData = ref(true);

onMounted(async () => {
    const [dmg, cond] = await Promise.all([
        loadDnDData('damageResImuVul.json'),
        loadDnDData('conditionResImuVul.json')
    ]);
    damageOptions.value = dmg || [];
    conditionOptions.value = cond || [];
    isLoadingData.value = false;
});

// --- Computed Properties für v-model ---
// Verwenden eine Funktion, um Wiederholungen zu vermeiden
function createListComputed(key) { // key ist 'resistances', 'immunities', etc.
    return computed({
        get: () => props.modelValue?.[key] ?? [],
        set: (value) => {
            const newValue = Array.isArray(value) ? value : (value ? [value] : []);
            emit('update:field', { key: key, value: newValue });
        }
    });
}

const resistances = createListComputed('resistances');
const immunities = createListComputed('immunities');
const vulnerabilities = createListComputed('vulnerabilities');
const conditionImmunities = createListComputed('conditionImmunities');

</script>

<template>
    <div>
        <div v-if="isLoadingData">Loading options...</div>
        <v-row v-else dense>
            <v-col cols="12">
                 <v-combobox
                    label="Damage Resistances"
                    v-model="resistances"
                    :items="damageOptions"
                    multiple chips closable-chips deletable-chips
                    density="compact" variant="outlined"
                    :disabled="!isEnabled"
                    hide-details="auto"
                 />
            </v-col>
            <v-col cols="12">
                 <v-combobox
                    label="Damage Immunities"
                    v-model="immunities"
                    :items="damageOptions"
                    multiple chips closable-chips deletable-chips
                    density="compact" variant="outlined"
                    :disabled="!isEnabled"
                    hide-details="auto"
                    class="mt-2"
                 />
            </v-col>
             <v-col cols="12">
                 <v-combobox
                    label="Damage Vulnerabilities"
                    v-model="vulnerabilities"
                    :items="damageOptions"
                    multiple chips closable-chips deletable-chips
                    density="compact" variant="outlined"
                    :disabled="!isEnabled"
                    hide-details="auto"
                    class="mt-2"
                 />
            </v-col>
             <v-col cols="12">
                 <v-combobox
                    label="Condition Immunities"
                    v-model="conditionImmunities"
                    :items="conditionOptions"
                    multiple chips closable-chips deletable-chips
                    density="compact" variant="outlined"
                    :disabled="!isEnabled"
                    hide-details="auto"
                    class="mt-2"
                 />
            </v-col>
        </v-row>
    </div>
</template>

<style scoped>
/* Optional: Styles */
</style>