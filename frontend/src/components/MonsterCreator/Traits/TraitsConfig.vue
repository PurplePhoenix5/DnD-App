<script setup>
import { ref, onMounted, computed } from 'vue'; // Remove watch import
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
import { VMenu } from 'vuetify/components/VMenu'; // Add VMenu import

const props = defineProps({
  modelValue: { type: Array, required: true },
  isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']);

// --- Lade Daten für Templates und Reset Types ---
const traitTemplates = ref([]); // Format: [{ id: '...', name: '...' }]
const resetTypes = ref([]);
const isLoadingData = ref(true);

function emitUpdate(newTraitsArray) {
    // Stellen Sie sicher, dass Sie immer ein Array emitten, auch wenn es leer ist
    emit('update:field', { key: 'traits', value: newTraitsArray || [] });
}

function createEmptyTraitObject() {
    // Stelle sicher, dass limitedUse Objekt immer da ist, auch wenn count 0 ist
    return { "name": "", "limitedUse": { "count": 0, "rate": "day" }, "description": "" };
}

function updateTraitField(index, field, value) {
    // Benutzen Sie cloneDeep, um eine unabhängige Kopie zu erstellen
    const newTraits = cloneDeep(props.modelValue);
    if (newTraits[index]) {
        // Verwenden Sie lodash.set für tiefere Pfade wie 'limitedUse.count'
        set(newTraits[index], field, value);
        emitUpdate(newTraits);
    }
}

async function addTrait(templateId) {
    let newTraitData;

    if (templateId) { // Wenn ein Template ausgewählt wurde (templateId ist nicht null)
        try {
            const response = await fetch(`http://localhost:8080/api/templates/trait/${templateId}`);
            if (!response.ok) throw new Error('Failed to load template');
            const templateData = await response.json();

            // --- NEU: Prüfe ob Trait mit diesem Namen bereits existiert ---
            // Annahme: templateData hat ein 'name' Feld
            if (templateData.name) {
                 const existingTrait = props.modelValue.find(t => t.name?.trim() === templateData.name?.trim());
                 if (existingTrait) {
                     // Zeige Bestätigungsdialog
                     const confirmAddAnyway = confirm(`A trait named "${templateData.name}" already exists in this monster. Do you want to add another one from the template?`);
                     if (!confirmAddAnyway) {
                         // Wenn Benutzer abbricht, tue nichts und setze den Select zurück
                         // Der v-select wird automatisch zurückgesetzt, da @update:model-value verwendet wird
                         return;
                     }
                 }
            }
             // --- Ende NEU ---


            // Verwende Template-Daten (ohne die Template-ID, falls vorhanden)
            // Sicherstellen, dass limitedUse existiert, falls es im Template fehlt
            newTraitData = {
                ...templateData,
                id: undefined, // Entferne Template-ID
                limitedUse: { // Stelle sicher, limitedUse Struktur existiert
                    count: templateData.limitedUse?.count ?? 0, // Standard 0, falls nicht im Template
                    rate: templateData.limitedUse?.rate ?? 'day' // Standard 'day', falls nicht im Template
                }
            };

        } catch (error) {
            console.error("Error loading trait template:", error);
            alert("Could not load the selected template.");
             // Auch hier ggf. Select zurücksetzen, obwohl Fehler aufgetreten ist
             // Ist aber weniger kritisch als bei Abbruch durch User
            return; // Breche ab, wenn Template nicht geladen werden kann
        }
    } else { // templateId ist null -> Add Empty Trait
        newTraitData = createEmptyTraitObject();
         // Optional: Standardname für ein leeres Trait
         if (!newTraitData.name) {
              newTraitData.name = `New Trait ${props.modelValue.length + 1}`;
         }
    }

    // KORREKT: Erstelle neues Array mit alten Elementen und dem neuen Trait
    const newTraits = [...props.modelValue, newTraitData];
    emitUpdate(newTraits);

    // Optional: Setze den v-select nach Auswahl zurück,
    // indem Sie null als neues modelValue emittieren.
    // Da Sie @update:model-value verwenden und kein v-model binden,
    // handhabt Vuetify den Reset in diesem Fall normalerweise selbst.
    // Wenn es nicht automatisch zurückgesetzt wird, müssten Sie ein
    // eigenes `selectedTemplate` ref einführen und dieses beim v-select binden.
    // Für diese Implementierung ist es wahrscheinlich nicht nötig.
}


function removeTrait(index) {
    // Erlaube das Löschen des letzten Traits
    const newTraits = props.modelValue.filter((_, i) => i !== index);
    emitUpdate(newTraits);
    // Das Template zeigt automatisch die "No traits added yet" Meldung an, wenn das Array leer ist.
}

function moveTrait(index, direction) {
    // KORREKT: Kopiere das Array
    const newTraits = [...props.modelValue];
    const newIndex = index + direction;
    if (newIndex < 0 || newIndex >= newTraits.length) return; // Ungültiger Index

    // Tausche Elemente mit Array Destructuring
    [newTraits[index], newTraits[newIndex]] = [newTraits[newIndex], newTraits[index]];

    emitUpdate(newTraits);
}

async function fetchTraitTemplates() {
    try {
        // Die API List-Route (GET /api/templates/trait) gibt die Summary zurück
        const response = await fetch('http://localhost:8080/api/templates/trait');
        if (!response.ok) throw new Error('Failed to fetch templates');
        return await response.json(); // Sollte ein Array von { id, name } zurückgeben
    } catch (error) {
        console.error("Error fetching trait templates:", error);
        return []; // Leeres Array bei Fehler
    }
}

// Die saveTraitAsTemplate und deleteTraitTemplate Funktionen sahen korrekt aus.
// Stelle sicher, dass die ID-Generierung genau mit dem Backend übereinstimmt.
async function saveTraitAsTemplate(index) {
    const trait = props.modelValue[index];
    if (!trait || !trait.name?.trim()) {
        alert("Trait needs a name to be saved as a template.");
        return;
    }

    // Erstelle ID genau wie im Backend
     let templateId = trait.name;
     // Prüfe sicher auf limitedUse Objekt und count Eigenschaft
     if (trait.limitedUse && typeof trait.limitedUse.count === 'number' && trait.limitedUse.count > 0) {
         templateId += "_uses" + trait.limitedUse.count;
     }
     // Sanitisiere ID
     templateId = templateId.toLowerCase();
     // Ersetze nicht-alphanumerische Zeichen (außer Unterstrich) durch Unterstrich
     templateId = templateId.replace(/[^a-z0-9]+/g, '_');
     // Entferne führende/nachgestellte Unterstriche
     templateId = templateId.replace(/^_+|_+$/g, '');
     // Entferne doppelte Unterstriche, falls vorhanden
     templateId = templateId.replace(/__+/g, '_');


    // Daten für POST (sollte die Trait-Struktur sein, ohne frontend-spezifische IDs etc.)
    // Stelle sicher, dass limitedUse Objekt komplett gesendet wird
    const templateData = {
        name: trait.name,
        limitedUse: {
            count: trait.limitedUse?.count ?? 0,
            rate: trait.limitedUse?.rate ?? 'day' // Sende Default, falls nicht gesetzt
        },
        description: trait.description ?? '' // Sende leeren String, falls nicht gesetzt
        // Füge hier keine ID hinzu, die generiert das Backend
    };


    try {
        const response = await fetch('http://localhost:8080/api/templates/trait', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(templateData) // Sende die vorbereiteten Daten
        });
        if (response.status === 409) {
            alert(`Error: A template with the generated ID "${templateId}" already exists (Name: "${trait.name}", Uses: ${trait.limitedUse?.count ?? 0}).`);
        } else if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
        } else {
             // Response body enthält das gespeicherte Template inkl. ID
             const savedTemplate = await response.json();
             alert(`Template "${savedTemplate.name}" (ID: ${savedTemplate.id}) saved successfully!`);
             // Lade Template-Liste neu, um die Add/Delete-Listen zu aktualisieren
             traitTemplates.value = await fetchTraitTemplates();
        }
    } catch (error) {
         console.error("Error saving trait template:", error);
         alert(`Error saving template: ${error.message}`);
    }
}

async function deleteTraitTemplate(templateId) {
    if (!templateId) {
         alert("No template ID provided for deletion.");
         return;
    }
    // Finde den Namen für die Bestätigung, falls möglich
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
         // Bei Erfolg 204 No Content, daher keine JSON-Antwort erwartet
         alert(`Template ${templateName} deleted successfully!`);
         // Lade Template-Liste neu
         traitTemplates.value = await fetchTraitTemplates();

     } catch (error) {
          console.error("Error deleting trait template:", error);
          alert(`Error deleting template: ${error.message}`);
     }
}


onMounted(async () => {
    isLoadingData.value = true; // Setze Ladezustand auf true beim Start
    try {
         // Lade Templates und Reset Types parallel
        const [templates, resets] = await Promise.all([
            fetchTraitTemplates(),
            loadDnDData('resetTypes.json')
        ]);
        traitTemplates.value = templates || [];
        resetTypes.value = resets || []; // Stelle sicher, dass es ein Array ist
    } catch (error) {
         console.error("Error loading initial data for TraitsConfig:", error);
         // Handle errors (z.B. Fehlermeldung anzeigen)
         resetTypes.value = []; // Setze auf leeres Array bei Fehler
         traitTemplates.value = []; // Setze auf leeres Array bei Fehler
    } finally {
        isLoadingData.value = false; // Setze Ladezustand auf false
    }
});


// Computed Property für das Add-Select
const templateOptions = computed(() => {
    const options = traitTemplates.value.map(t => ({ title: t.name, value: t.id }));
    // Füge leeres Template als erste Option hinzu
    // value: null ist korrekt und wird von addTrait(null) verarbeitet
    options.unshift({ title: '--- Add Empty Trait ---', value: null });
    return options;
});

</script>

<template>
    <div :class="{'disabled-content': !isEnabled}">
        <div v-if="isLoadingData" class="pa-4 text-center text-disabled">Loading data...</div>
        <div v-else-if="modelValue.length === 0" class="pa-4 text-center text-disabled">
            No traits added yet. Use the "Add Trait" button below.
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
                            <!-- Stellen Sie sicher, dass Uses immer ein Integer >= 0 ist -->
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
                            />
                        </v-col>

                        <!-- Zeile 2 & 3: Description -->
                         <v-col cols="12">
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
                            <!-- Erlaube das Löschen, auch wenn es das letzte Trait ist -->
                            <v-btn @click="removeTrait(index)" color="error" variant="outlined" :disabled="!isEnabled" block>Delete Trait</v-btn>
                        </v-col>
                         <v-col cols="12" md="2">
                             <!-- Deaktiviere Speichern, wenn Name leer ist -->
                            <v-btn @click="saveTraitAsTemplate(index)" color="secondary" variant="outlined" :disabled="!isEnabled || !trait.name?.trim()" block>Save Template</v-btn>
                        </v-col>
                         <v-col cols="12" md="2">
                             <v-menu>
                                 <template v-slot:activator="{ props: menuProps }">
                                     <!-- Deaktiviere "Delete Template" Button, wenn keine Templates geladen sind -->
                                      <v-btn color="secondary" variant="outlined" :disabled="!isEnabled || traitTemplates.length === 0" block v-bind="menuProps">Delete Template</v-btn>
                                 </template>
                                 <v-list density="compact">
                                     <!-- Zeige Liste der geladenen Templates -->
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

         <!-- Add Trait Button/Select -->
         <v-select
           label="Add Trait"
           :items="templateOptions"
           @update:model-value="addTrait($event)"
           density="compact" variant="outlined"
           :disabled="!isEnabled || isLoadingData"
           hide-details
           class="mt-4"
           no-data-text="No templates found"
           clearable
           placeholder="Select template or add empty"
           :model-value="null"
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