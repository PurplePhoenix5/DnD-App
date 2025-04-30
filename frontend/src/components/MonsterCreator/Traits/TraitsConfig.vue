<script setup>
import { ref, onMounted, computed, watch } from 'vue'; // 'watch' wird jetzt für selectedTemplate benötigt, also wieder importieren
import { cloneDeep, set } from 'lodash';
import { loadDnDData } from '../../../utils/dndDataService.js';
import { VTextarea } from 'vuetify/components/VTextarea';
import { VNumberInput } from 'vuetify/lib/components/index.mjs';
import { VSelect } from 'vuetify/components/VSelect';
import { VBtn } from 'vuetify/components/VBtn';
import { VIcon } from 'vuetify/components/VIcon';
import { VExpansionPanels, VExpansionPanel, VExpansionPanelTitle, VExpansionPanelText } from 'vuetify/components/VExpansionPanel';
import { VDivider } from 'vuetify/components/VDivider';
import { VRow, VCol } from 'vuetify/components/VGrid';
import { VList, VListItem, VListItemTitle } from 'vuetify/components/VList';
import { VMenu } from 'vuetify/components/VMenu';

const props = defineProps({
  modelValue: { type: Array, required: true },
  isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']);

// --- Lade Daten für Templates und Reset Types ---
const traitTemplates = ref([]);
const resetTypes = ref([]);
const isLoadingData = ref(true);

function emitUpdate(newTraitsArray) {
    emit('update:field', { key: 'traits', value: newTraitsArray || [] });
}

function createEmptyTraitObject() {
    return { "name": "", "limitedUse": { "count": 0, "rate": "day" }, "description": "" };
}

function updateTraitField(index, field, value) {
    const newTraits = cloneDeep(props.modelValue);
    if (newTraits[index]) {
        set(newTraits[index], field, value);
        emitUpdate(newTraits);
    }
}

function addNewEmptyTrait() {
    if (!props.isEnabled) return;

    const newTraits = cloneDeep(props.modelValue);
    const newTraitData = createEmptyTraitObject();
     if (!newTraitData.name) {
          newTraitData.name = `New Trait ${newTraits.length + 1}`;
     }

    newTraits.push(newTraitData);
    emitUpdate(newTraits);
}

async function addTraitFromTemplate(templateId) {
    if (!props.isEnabled || !templateId) return;

    try {
        const response = await fetch(`http://localhost:8080/api/templates/trait/${templateId}`);
        if (!response.ok) {
             const errorText = await response.text();
             throw new Error(`Failed to load template (Status: ${response.status}). Response: ${errorText}`);
        }
        const templateData = await response.json();

        if (templateData.name) {
             const existingTrait = props.modelValue.find(t => t.name?.trim() === templateData.name?.trim());
             if (existingTrait) {
                 const confirmAddAnyway = confirm(`A trait named "${templateData.name}" already exists in this monster. Do you want to add another one from the template?`);
                 if (!confirmAddAnyway) {
                     return;
                 }
             }
        }

        const newTraitData = {
            ...templateData,
            id: undefined,
             limitedUse: {
                 count: templateData.limitedUse?.count ?? 0,
                 rate: templateData.limitedUse?.rate ?? 'day'
            },
            description: templateData.description ?? ''
        };

        const newTraits = [...props.modelValue, newTraitData];
        emitUpdate(newTraits);

    } catch (error) {
        console.error("Error loading trait template:", error);
        alert(`Could not load the selected template: ${error.message}`);
    } finally {
         // Setze den Select nach Auswahl oder Fehler zurück
         selectedTemplate.value = null;
    }
}


function removeTrait(index) {
    if (!props.isEnabled) return;
    const newTraits = props.modelValue.filter((_, i) => i !== index);
    emitUpdate(newTraits);
}

function moveTrait(index, direction) {
    if (!props.isEnabled) return;
    const newTraits = [...props.modelValue];
    const newIndex = index + direction;
    if (newIndex < 0 || newIndex >= newTraits.length) return;

    [newTraits[index], newTraits[newIndex]] = [newTraits[newIndex], newTraits[index]];

    emitUpdate(newTraits);
}

async function fetchTraitTemplates() {
    try {
        const response = await fetch('http://localhost:8080/api/templates/trait');
        if (!response.ok) throw new Error('Failed to fetch templates');
        return await response.json();
    } catch (error) {
        console.error("Error fetching trait templates:", error);
        return [];
    }
}

async function saveTraitAsTemplate(index) {
    if (!props.isEnabled) return;
    const trait = props.modelValue[index];
    if (!trait || !trait.name?.trim()) {
        alert("Trait needs a name to be saved as a template.");
        return;
    }

     let templateId = trait.name;
     if (trait.limitedUse && typeof trait.limitedUse.count === 'number' && trait.limitedUse.count > 0) {
         templateId += "_uses" + trait.limitedUse.count;
     }
     templateId = templateId.toLowerCase();
     templateId = templateId.replace(/[^a-z0-9]+/g, '_');
     templateId = templateId.replace(/^_+|_+$/g, '');
     templateId = templateId.replace(/__+/g, '_');


    const templateData = {
        name: trait.name,
        limitedUse: {
            count: trait.limitedUse?.count ?? 0,
            rate: trait.limitedUse?.rate ?? 'day'
        },
        description: trait.description ?? ''
    };


    try {
        const response = await fetch('http://localhost:8080/api/templates/trait', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(templateData)
        });
        if (response.status === 409) {
            alert(`Error: A template with the generated ID "${templateId}" already exists (Name: "${trait.name}", Uses: ${trait.limitedUse?.count ?? 0}).`);
        } else if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
        } else {
             const savedTemplate = await response.json();
             alert(`Template "${savedTemplate.name}" (ID: ${savedTemplate.id}) saved successfully!`);
             traitTemplates.value = await fetchTraitTemplates();
        }
    } catch (error) {
         console.error("Error saving trait template:", error);
         alert(`Error saving template: ${error.message}`);
    }
}

async function deleteTraitTemplate(templateId) {
    if (!props.isEnabled) return;
    if (!templateId) {
         alert("No template ID provided for deletion.");
         return;
    }
    const templateToDelete = traitTemplates.value.find(t => t.id === templateId);
    const templateName = templateToDelete ? `"${templateToDelete.name}" (ID: ${templateId})` : `ID: ${templateId}`;

    if (!confirm(`Are you sure you want to delete the template ${templateName}? This cannot be undone.`)) {
        return;
    }

     try {
        const response = await fetch(`http://localhost:8080/api/templates/trait/${templateId}`, {
            method: 'DELETE'
        });
         if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
         }
         alert(`Template ${templateName} deleted successfully!`);
         traitTemplates.value = await fetchTraitTemplates();

     } catch (error) {
          console.error("Error deleting trait template:", error);
          alert(`Error deleting template: ${error.message}`);
     }
}


onMounted(async () => {
    isLoadingData.value = true;
    try {
        const [templates, resets] = await Promise.all([
            fetchTraitTemplates(),
            loadDnDData('resetTypes.json')
        ]);
        traitTemplates.value = templates || [];
        resetTypes.value = resets || [];
    } catch (error) {
         console.error("Error loading initial data for TraitsConfig:", error);
         resetTypes.value = [];
         traitTemplates.value = [];
    } finally {
        isLoadingData.value = false;
    }
});

// Nur Templates für das Select
const templateOptions = computed(() => {
    return traitTemplates.value.map(t => ({ title: t.name, value: t.id }));
});

// Lokaler Ref für v-model des Selects und Watcher
const selectedTemplate = ref(null);

watch(selectedTemplate, (newValue) => {
    if (newValue !== null) {
        addTraitFromTemplate(newValue);
        // selectedTemplate wird jetzt in addTraitFromTemplate().finally gesetzt
    }
});

</script>

<template>
    <div :class="{'disabled-content': !isEnabled}">
        <div v-if="isLoadingData" class="pa-4 text-center text-disabled">Loading data...</div>
        <div v-else-if="modelValue.length === 0" class="pa-4 text-center text-disabled">
            No traits added yet. Use the "Add Empty Trait" button below.
        </div>
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
                            <v-number-input
                                label="Uses"
                                :model-value="trait.limitedUse?.count"
                                @update:model-value="updateTraitField(index, 'limitedUse.count', parseInt($event, 10) || 0)"
                                density="compact" variant="outlined"
                                :min="0"
                                control-variant="stacked"
                                :reverse="false"
                                inset
                                :disabled="!isEnabled"
                            />
                        </v-col>
                        <v-col cols="12" md="4">
                            <v-select
                                label="Reset Type"
                                :items="resetTypes.map(item => ({ title: item.name, value: item.id }))"
                                :model-value="trait.limitedUse?.rate"
                                @update:model-value="updateTraitField(index, 'limitedUse.rate', $event)"
                                density="compact" variant="outlined"
                                clearable
                                :disabled="!isEnabled || (trait.limitedUse?.count ?? 0) === 0"
                                item-title="title"  
                                item-value="value" 
                            />
                        </v-col>

                        <!-- Zeile 2 & 3: Description -->
                         <v-col cols="12">
                             <v-textarea
                                label="Description (Markdown: **Bold** and _Italic_)"
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
                            <v-btn @click="saveTraitAsTemplate(index)" color="secondary" variant="outlined" :disabled="!isEnabled || !trait.name?.trim()" block>Save Template</v-btn>
                        </v-col>
                         <v-col cols="12" md="2">
                             <v-menu>
                                 <template v-slot:activator="{ props: menuProps }">
                                      <v-btn color="secondary" variant="outlined" :disabled="!isEnabled || traitTemplates.length === 0" block v-bind="menuProps">Delete Template</v-btn>
                                 </template>
                                 <v-list density="compact">
                                     <v-list-item
                                         v-for="template in traitTemplates"
                                         :key="template.id"
                                         @click="deleteTraitTemplate(template.id)"
                                         :title="template.name"
                                         density="compact"
                                     >
                                         <template v-slot:append><v-icon icon="mdi-delete" size="small"></v-icon></template>
                                     </v-list-item>
                                     <v-list-item v-if="traitTemplates.length === 0" disabled title="No templates available"></v-list-item>
                                 </v-list>
                             </v-menu>
                         </v-col>
                    </v-row>
                </v-expansion-panel-text>
            </v-expansion-panel>
        </v-expansion-panels>

         <!-- NEUE UI für Add Button und Add Template Select -->
         <v-row dense class="mt-4">
             <v-col cols="12" md="6">
                 <v-btn
                     block
                     color="primary"
                     variant="outlined"
                     @click="addNewEmptyTrait"
                     :disabled="!isEnabled"
                 >
                     Add Empty Trait
                 </v-btn>
             </v-col>
             <v-col cols="12" md="6">
                 <v-select
                   label="Add Trait from Template"
                   :items="templateOptions"
                   v-model="selectedTemplate"
                   density="compact" variant="outlined"
                   :disabled="!isEnabled || isLoadingData || templateOptions.length === 0"
                   hide-details
                   no-data-text="No templates available"
                   clearable
                   placeholder="Select a template"
                 >
                 </v-select>
             </v-col>
         </v-row>
         <!-- ENDE NEUE UI -->

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