<!-- frontend/src/components/MonsterCreator/Traits/TraitsConfig.vue -->
<script setup>
import { ref, onMounted, computed, watch } from 'vue';
import { cloneDeep, set } from 'lodash';
import { loadDnDData } from '../../../utils/dndDataService.js';
import { VTextarea } from 'vuetify/components/VTextarea'; // Importiere VTextarea
import { VNumberInput } from 'vuetify/lib/components/index.mjs'; // Expliziter Import für VNumberInput
import { VSelect } from 'vuetify/components/VSelect';
import { VBtn } from 'vuetify/components/VBtn';
import { VIcon } from 'vuetify/components/VIcon';
import { VTooltip } from 'vuetify/components/VTooltip';
import { VExpansionPanels, VExpansionPanel, VExpansionPanelTitle, VExpansionPanelText } from 'vuetify/components/VExpansionPanel';
import { VDivider } from 'vuetify/components/VDivider';
import { VRow, VCol } from 'vuetify/components/VGrid';
import { VList, VListItem, VListItemTitle } from 'vuetify/components/VList'; // Für Select Item

const props = defineProps({
  modelValue: { type: Array, required: true }, // Erhält das 'traits'-Array
  isEnabled: { type: Boolean, default: false }
});
const emit = defineEmits(['update:field']);

// --- Lade Daten für Templates und Reset Types ---
const traitTemplates = ref([]); // Format: [{ id: '...', name: '...' }]
const resetTypes = ref([]);
const isLoadingData = ref(true);

onMounted(async () => {
    const [templates, resets] = await Promise.all([
        fetchTraitTemplates(), // Eigene Funktion für Templates
        loadDnDData('resetTypes.json')
    ]);
    traitTemplates.value = templates || [];
    resetTypes.value = resets || [];
    isLoadingData.value = false;
});

async function fetchTraitTemplates() {
    try {
        const response = await fetch('http://localhost:8080/api/templates/trait');
        if (!response.ok) throw new Error('Failed to fetch templates');
        return await response.json();
    } catch (error) {
        console.error("Error fetching trait templates:", error);
        return []; // Leeres Array bei Fehler
    }
}

// --- Methoden ---
function updateTraitArray(newTraitsArray) {
    emit('update:field', { key: 'traits', value: newTraitsArray });
}

function updateTraitField(index, field, value) {
    const newTraits = cloneDeep(props.modelValue);
    if (newTraits[index]) {
        // Benutze set für potenziell verschachtelte Felder wie limitedUse.count
        set(newTraits[index], field, value);
        updateTraitArray(newTraits);
    }
}

async function addTrait(templateId) {
    let newTraitData = { name: '', limitedUse: { count: 0, rate: 'day' }, description: '' }; // Leeres Trait

    if (templateId) { // Wenn ein Template ausgewählt wurde
        try {
            const response = await fetch(`http://localhost:8080/api/templates/trait/${templateId}`);
            if (!response.ok) throw new Error('Failed to load template');
            const templateData = await response.json();
            // Überschreibe leeres Trait mit Template-Daten (ohne ID)
            newTraitData = { ...newTraitData, ...templateData, id: undefined };
        } catch (error) {
            console.error("Error loading trait template:", error);
            alert("Could not load the selected template.");
            return; // Breche ab, wenn Template nicht geladen werden kann
        }
    } else {
        // Füge leeres Template hinzu
         newTraitData.name = "New Trait"; // Gib einen Standardnamen
    }


    const newTraits = cloneDeep(props.modelValue);
    newTraits.push(newTraitData);
    updateTraitArray(newTraits);
}

function removeTrait(index) {
    const newTraits = cloneDeep(props.modelValue);
    newTraits.splice(index, 1);
    updateTraitArray(newTraits);
}

function moveTrait(index, direction) {
    const newTraits = cloneDeep(props.modelValue);
    const newIndex = index + direction;
    if (newIndex < 0 || newIndex >= newTraits.length) return; // Ungültiger Index

    // Tausche Elemente
    [newTraits[index], newTraits[newIndex]] = [newTraits[newIndex], newTraits[index]];
    updateTraitArray(newTraits);
}

// --- Template Speichern/Löschen ---
async function saveTraitAsTemplate(index) {
    const trait = props.modelValue[index];
    if (!trait || !trait.name?.trim()) {
        alert("Trait needs a name to be saved as a template.");
        return;
    }

    // Erstelle ID (wie im Backend)
     let templateId = trait.name;
     if (trait.limitedUse?.count) {
         templateId += "_uses" + trait.limitedUse.count;
     }
     templateId = templateId.toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/^_|_$/g, '');


    // Daten für POST (ohne generierte ID)
    const templateData = { ...trait, id: undefined };

    try {
        const response = await fetch('http://localhost:8080/api/templates/trait', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(templateData)
        });
        if (response.status === 409) {
            alert(`Error: A template with ID "${templateId}" already exists.`);
        } else if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
        } else {
             alert(`Template "${trait.name}" saved successfully!`);
             // Lade Template-Liste neu
             traitTemplates.value = await fetchTraitTemplates();
        }
    } catch (error) {
         console.error("Error saving trait template:", error);
         alert(`Error saving template: ${error.message}`);
    }
}

async function deleteTraitTemplate(templateId) {
    if (!templateId) return;
    if (!confirm(`Are you sure you want to delete the template "${templateId}"? This cannot be undone.`)) return;

     try {
        const response = await fetch(`http://localhost:8080/api/templates/trait/${templateId}`, {
            method: 'DELETE'
        });
         if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
         }
         alert(`Template "${templateId}" deleted successfully!`);
         // Lade Template-Liste neu
         traitTemplates.value = await fetchTraitTemplates();

     } catch (error) {
          console.error("Error deleting trait template:", error);
          alert(`Error deleting template: ${error.message}`);
     }
}

// Computed Property für das Add-Select
const templateOptions = computed(() => {
    const options = traitTemplates.value.map(t => ({ title: t.name, value: t.id }));
    // Füge leeres Template als erste Option hinzu
    options.unshift({ title: '--- Add Empty Trait ---', value: null });
    return options;
});

</script>

<template>
    <div :class="{'disabled-content': !isEnabled}"> <!-- Deaktiviere Inhalt visuell -->
        <div v-if="isLoadingData">Loading data...</div>
        <v-expansion-panels v-else variant="accordion" multiple>
            <v-expansion-panel v-for="(trait, index) in modelValue" :key="index" elevation="1">
                <v-expansion-panel-title :disabled="!isEnabled">
                     <!-- Zeige Trait-Namen -->
                    <span class="flex-grow-1 mr-2">{{ trait.name || `Trait ${index + 1}` }}</span>
                    <!-- Move Buttons -->
                    <v-btn icon="mdi-arrow-up-bold-box-outline" size="x-small" variant="text" @click.stop="moveTrait(index, -1)" :disabled="index === 0 || !isEnabled" class="mr-1"/>
                    <v-btn icon="mdi-arrow-down-bold-box-outline" size="x-small" variant="text" @click.stop="moveTrait(index, 1)" :disabled="index === modelValue.length - 1 || !isEnabled"/>
                </v-expansion-panel-title>
                <v-expansion-panel-text>
                    <v-row dense>
                        <!-- Zeile 1: Name, Uses, Reset -->
                        <v-col cols="12" md="6">
                             <v-text-field label="Name" :model-value="trait.name" @update:model-value="updateTraitField(index, 'name', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled"/>
                        </v-col>
                        <v-col cols="12" md="2">
                            <v-number-input label="Uses" :model-value="trait.limitedUse?.count" @update:model-value="updateTraitField(index, 'limitedUse.count', parseInt($event, 10) || 0)" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled"/>
                        </v-col>
                        <v-col cols="12" md="4">
                            <v-select label="Reset Type" :items="resetTypes" :model-value="trait.limitedUse?.rate" @update:model-value="updateTraitField(index, 'limitedUse.rate', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled || (trait.limitedUse?.count ?? 0) === 0"/>
                        </v-col>

                        <!-- Zeile 2 & 3: Description -->
                         <v-col cols="12">
                             <!-- HINWEIS: v-textarea unterstützt kein direktes Fett/Kursiv.
                                  Du bräuchtest einen Rich-Text-Editor (z.B. Tiptap/Vuetify Extension
                                  oder Quill) oder müsstest Markdown verwenden und es im Renderer umwandeln.
                                  Vorerst ein normales Textarea. -->
                             <v-textarea
                                label="Description (Markdown or HTML allowed)"
                                :model-value="trait.description"
                                @update:model-value="updateTraitField(index, 'description', $event)"
                                density="compact" variant="outlined"
                                rows="4" auto-grow
                                :disabled="!isEnabled"
                              />
                         </v-col>

                        <!-- Zeile 4: Buttons -->
                        <v-col cols="12" md="8">
                            <v-btn @click="removeTrait(index)" color="error" variant="outlined" :disabled="!isEnabled" block>Delete Trait</v-btn>
                        </v-col>
                         <v-col cols="12" md="2">
                            <v-btn @click="saveTraitAsTemplate(index)" color="secondary" variant="outlined" :disabled="!isEnabled || !trait.name" block>Save Template</v-btn>
                        </v-col>
                         <v-col cols="12" md="2">
                             <v-menu>
                                 <template v-slot:activator="{ props: menuProps }">
                                      <v-btn color="secondary" variant="outlined" :disabled="!isEnabled || traitTemplates.length === 0" block v-bind="menuProps">Delete Template</v-btn>
                                 </template>
                                 <v-list density="compact">
                                     <v-list-item v-for="template in traitTemplates" :key="template.id" @click="deleteTraitTemplate(template.id)" :title="template.name" density="compact">
                                         <template v-slot:append><v-icon icon="mdi-delete" size="small"></v-icon></template>
                                     </v-list-item>
                                 </v-list>
                             </v-menu>
                         </v-col>
                    </v-row>
                </v-expansion-panel-text>
            </v-expansion-panel>
        </v-expansion-panels>

         <!-- Add Trait Button/Select -->
         <v-select
           label="Add Trait"
           :items="templateOptions"
           @update:model-value="addTrait($event); $event = null" 
           density="compact" variant="outlined"
           :disabled="!isEnabled || isLoadingData"
           hide-details
           class="mt-4"
           no-data-text="No templates found"
           clearable
           placeholder="Select template or add empty"
         >
             <template v-slot:item="{ props: itemProps, item }">
                <v-list-item v-bind="itemProps" :title="item.title"></v-list-item>
                <!-- Trennlinie nach "Add Empty" -->
                <v-divider v-if="item.value === null"></v-divider>
            </template>
         </v-select>
    </div>
</template>

<style scoped>
.disabled-content {
    opacity: 0.6;
    pointer-events: none; /* Verhindert Klicks auf deaktivierten Inhalt */
}
/* Style für linksbündigen Input in v-number-input (falls noch benötigt) */
:deep(.v-number-input .v-field__input) {
  text-align: left !important;
}
</style>