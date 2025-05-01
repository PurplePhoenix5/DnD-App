<!-- frontend/src/components/MonsterCreator/Actions/ActionConfigBase.vue -->
<script setup>
import { ref, onMounted, computed, watch } from 'vue';
import { cloneDeep, set, get } from 'lodash';
import { loadDnDData } from '../../../utils/dndDataService.js'; // Für allgemeine DnD Daten
import { avgRoll, calculateSaveDC } from '../../../utils/mathRendering.js'; // Für Berechnungen
import { VExpansionPanels, VExpansionPanel, VExpansionPanelTitle, VExpansionPanelText } from 'vuetify/components/VExpansionPanel';
import { VRow, VCol } from 'vuetify/components/VGrid';
import { VTextField } from 'vuetify/components/VTextField';
import { VNumberInput } from 'vuetify/lib/components/index.mjs';
import { VSelect } from 'vuetify/components/VSelect';
import { VCombobox } from 'vuetify/components/VCombobox'; // Für Damage Type
import { VBtn } from 'vuetify/components/VBtn';
import { VIcon } from 'vuetify/components/VIcon';
import { VTooltip } from 'vuetify/components/VTooltip';
import { VBtnToggle } from 'vuetify/components/VBtnToggle';
import { VMenu } from 'vuetify/components/VMenu'; // Für Delete Template Liste
import { VList, VListItem, VListItemTitle } from 'vuetify/components/VList'; // Für Delete Template Liste
import { VDivider } from 'vuetify/components/VDivider'; // Für Trenner

const props = defineProps({
  modelValue: { type: Array, required: true }, // Array von Action-Objekten (AttackRoll, SavingThrow, Other gemischt)
  actionGroupType: { type: String, required: true, validator: val => ['actions', 'bonusAction'].includes(val) }, // 'actions' oder 'bonusAction'
  basicsData: { type: Object, required: true }, // Für Save DC Berechnung (braucht Stats, PB)
  isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']);

// --- State für Datenladen ---
const isLoadingData = ref(true);
const attackRanges = ref([]);
const diceOptions = ref([]); // Für Die Size Select
const damageTypes = ref([]); // Für Damage Type Select
const rechargeOptions = ref([]);
const resetTypes = ref([]); // Für Reset Type Select
const statsOptions = ref([]); // Für Saving Throw Stat Select

// --- State für Add Bar ---
const selectedActionType = ref('attackRoll'); // Standardmäßig Attack Roll
const selectedTemplate = ref(null);
const actionTemplates = ref([]); // Geladene Templates für den selectedActionType

// --- State für Damage Input Reihen (innerhalb Attack Roll & Saving Throw) ---
// Lokale Refs für temporäre Damage Input Werte, wenn eine neue Damage Reihe hinzugefügt wird
// Diese werden auf null/'' gesetzt, nachdem die Reihe zur action.damage Liste hinzugefügt wurde
const newDamageInput = ref({ count: null, size: null, modifier: null, type: null, averageDamage: null });


// --- Watcher für selectedActionType zum Laden von Templates ---
watch(selectedActionType, async (newType) => {
    selectedTemplate.value = null; // Select zurücksetzen
    if (!newType) {
        actionTemplates.value = [];
        return;
    }
    console.log(`Fetching templates for type: ${newType}`);
    actionTemplates.value = await fetchActionTemplates(newType);
}, { immediate: true }); // Sofort beim Laden der Komponente auslösen

// --- Computed Property für Template Select Items ---
const templateOptions = computed(() => {
    return actionTemplates.value.map(t => ({ title: t.name, value: t.id }));
});

// --- Watcher für selectedTemplate zum Hinzufügen vom Template ---
watch(selectedTemplate, (newValue) => {
    if (newValue !== null && selectedActionType.value) {
        addActionFromTemplate(selectedActionType.value, newValue);
        // selectedTemplate wird in addActionFromTemplate().finally zurückgesetzt
    }
});

function groupActionsByType(flatActionsArray) {
    // Stelle sicher, dass die Struktur existiert
    const grouped = {
        attackRoll: [],
        savingThrow: [],
        other: []
        // Füge hier weitere Typen hinzu, falls das Schema erweitert wird
    };
    (flatActionsArray || []).forEach(action => {
        // Kopiere das Action-Objekt und entferne die temporäre 'type' Eigenschaft
        const actionCopy = cloneDeep(action);
        const type = actionCopy.type; // Hole den Typ

        if (type && grouped[type]) {
            // Optional: Entferne die type-Eigenschaft, bevor sie zum Gruppenobjekt hinzugefügt wird,
            // falls das Backend-Schema diese Eigenschaft auf dieser Ebene nicht erwartet.
            // Laut Ihrem Schema-Ausschnitt haben attackRoll/savingThrow/other
            // Objekte in den Arrays *keine* type-Eigenschaft.
            delete actionCopy.type;

            grouped[type].push(actionCopy);
        } else {
            console.warn("Action object missing or has invalid type, skipping:", action);
        }
    });
    // console.log("Grouped actions for emission:", grouped);
    return grouped;
}

// --- Helpers ---
// Emittiert das gesamte Array (modelValue) an den Parent
function emitUpdate(newFlatActionsArray) {
    // Transformiere das flache Array zurück in die Objektstruktur
    const newGroupedActions = groupActionsByType(newFlatActionsArray);

    // Emittiere die Objektstruktur mit dem korrekten Schlüssel ('actions' oder 'bonusAction')
    // Der Pfad ist nur der Top-Level-Schlüssel
    emit('update:field', { key: props.actionGroupType, value: newGroupedActions });
}

// Aktualisiert ein Feld eines spezifischen Actions-Objekts im Array
function updateActionField(index, field, value) {
    const newActions = cloneDeep(props.modelValue);
    if (newActions[index]) {
        set(newActions[index], field, value);
        // Spezielle Behandlung für Uses/Reset/Recharge Abhängigkeit
        if (field === 'uses' || field === 'limitedUse.count' || field === 'limitedUse.rate' || field === 'recharge') {
            handleUsesRechargeDependency(newActions[index]);
        }
         // Spezielle Behandlung für Save Stat Änderung -> DC neuberechnen (nur bei Saving Throw)
         if (newActions[index].type === 'savingThrow' && field === 'saveStat') {
             // Berechne neuen Default DC und setze ihn, falls kein Override da ist
              const calculatedDc = calculateSaveDC(props.basicsData, value); // Nutze basicsData für Berechnung
              if (newActions[index].safeDC?.overrideValue === null) {
                   set(newActions[index], 'safeDC.defaultValue', calculatedDc);
              }
         }
        emitUpdate(newActions);
    }
}

// Fügt ein Feld zu einem verschachtelten Array (z.B. action.damage) hinzu
function addNestedArrayItem(index, arrayField, item) {
    const newActions = cloneDeep(props.modelValue);
    if (newActions[index]) {
        let currentArray = get(newActions[index], arrayField, []);
        // Stellen Sie sicher, dass es ein Array ist
        if (!Array.isArray(currentArray)) {
            console.warn(`Field ${arrayField} is not an array on action at index ${index}. Initializing as array.`);
            set(newActions[index], arrayField, []);
            currentArray = get(newActions[index], arrayField); // Holen Sie das neu initialisierte Array
        }
        currentArray.push(item);
        set(newActions[index], arrayField, currentArray); // Setze das aktualisierte Array
        emitUpdate(newActions);
    }
}

// Entfernt ein Element aus einem verschachtelten Array (z.B. action.damage)
function removeNestedArrayItem(actionIndex, arrayField, itemIndex) {
     const newActions = cloneDeep(props.modelValue);
     if (newActions[actionIndex]) {
        let currentArray = get(newActions[actionIndex], arrayField, []);
          if (Array.isArray(currentArray) && itemIndex >= 0 && itemIndex < currentArray.length) {
              currentArray.splice(itemIndex, 1);
             set(newActions[actionIndex], arrayField, currentArray);
             emitUpdate(newActions);
          } else {
              console.warn(`Cannot remove item ${itemIndex} from ${arrayField} on action at index ${actionIndex}. Array invalid or index out of bounds.`);
          }
     }
}

// Behandelt die Abhängigkeit zwischen Uses/Reset und Recharge
function handleUsesRechargeDependency(action) {
    // Diese Funktion wird auf einem *spezifischen Action-Objekt* ausgeführt
    // Passt die Disabled-States in der UI an, nicht die Daten selbst.
    // Die Logik zum Deaktivieren muss im Template mit Computed Properties oder
    // Funktionen implementiert werden, die Uses, Reset und Recharge prüfen.
    // Diese Funktion ist daher eher ein Hinweis, dass die UI-Logik existieren muss.
    console.log("Handling Uses/Recharge dependency logic for action:", action);
}

// Funktion zum Berechnen der angezeigten Average Damage + Modifier
function calculateDisplayedAverageDamage(count, dieSize, modifier) {
    const numCount = parseInt(count, 10);
    const numModifier = parseInt(modifier, 10);
    const dieValue = parseInt(dieSize, 10); // Verwende numerischen Wert aus diceOptions

    if (isNaN(numCount) || numCount <= 0 || isNaN(dieValue) || dieValue <= 0) {
        return '-'; // Kann nicht berechnet werden
    }

    const averageRoll = avgRoll(numCount, dieValue); // Nutze mathRendering avgRoll
    const totalAverage = averageRoll + (isNaN(numModifier) ? 0 : numModifier);

    return totalAverage.toString(); // Zeige als String
}

// Computed Property für das Mapping der diceOptions (z.B. { title: 'd6', value: 6 })
const mappedDiceOptions = computed(() => {
    if (!diceOptions.value || !diceOptions.value.diceMapping) return [];
    return Object.entries(diceOptions.value.diceMapping).map(([key, value]) => ({
        title: key, // z.B. 'd6'
        value: value // z.B. 6 (als Nummer für die Berechnung)
    }));
});

// --- Add Action Bar Logik ---

// Fügt ein neues leeres Action-Objekt hinzu
function addEmptyAction(type) {
    if (!props.isEnabled || !type) return;

    // === KORRIGIERT ===
    // Arbeite mit einer Kopie des FLACHEN modelValue Arrays
    const newActions = cloneDeep(props.modelValue);
    // ==================

    let newActionData = { type: type, name: 'New Action' }; // Basis mit Typ und Standardname

    // Fülle spezifische Default-Werte basierend auf dem Typ
    if (type === 'attackRoll') {
        newActionData = {
            ...newActionData,
            attackMod: 0,
            rangeType: 'melee',
            reachMelee: null,
            reachRanged: null,
            reachDisadvantage: null,
            damage: [], // Array für Damage-Einträge
            notes: ''
             // Zukünftige Felder hier initialisieren
        };
    } else if (type === 'savingThrow') {
         newActionData = {
             ...newActionData,
             saveStat: 'STR', // Standard-Stat
             safeDC: { defaultValue: 0, overrideValue: null }, // Standardstruktur für DC
             limitedUse: { count: null, rate: null }, // Optional Use/Reset
             recharge: null, // Optional Recharge
             notes: '', // Notes für den Haupt-Textblock (nicht Damage)
             damage: [], // Array für Damage-Einträge (Fail)
             successNotes: '', // Notizen bei Erfolg
             succesOrFailNotes: '' // Notizen bei Erfolg oder Misserfolg
         };
         // Berechne initialen Default DC, falls basicsData bereit und stat gesetzt
         if (props.basicsData?.stats && props.basicsData.PB !== undefined && newActionData.saveStat) {
             newActionData.safeDC.defaultValue = calculateSaveDC(props.basicsData, newActionData.saveStat);
         } else {
             console.warn("Cannot calculate initial Save DC for new Saving Throw action: Missing basicsData or stat.");
         }

    } else if (type === 'other') {
        newActionData = {
            ...newActionData,
             limitedUse: { count: null, rate: null }, // Optional Use/Reset
             recharge: null, // Optional Recharge
            description: ''
        };
    }

    // === KORRIGIERT ===
    // Füge das neue Action-Objekt zum FLACHEN Array hinzu
    newActions.push(newActionData);
    // Emittiere das aktualisierte FLATE Array (emitUpdate wandelt es um)
    emitUpdate(newActions);
    // ==================
}

// Fügt eine Action von einem Template hinzu
async function addActionFromTemplate(type, templateId) {
     if (!props.isEnabled || !type || !templateId) return;

     try {
         // Hole Template Daten vom spezifischen Endpunkt für den Typ
         const response = await fetch(`http://localhost:8080/api/templates/${type}/${templateId}`);
         if (!response.ok) {
             const errorText = await response.text();
             throw new Error(`Failed to load template (${type}, ID: ${templateId}). Status: ${response.status}). Response: ${errorText}`);
         }
         const templateData = await response.json();

         // Füge den Typ hinzu und entferne Template-ID
         const newActionData = {
              ...templateData,
              type: type, // Wichtig: Den Typ festlegen!
              id: undefined // Template-ID entfernen
             // Weitere Anpassungen/Defaults können hier erfolgen, falls nötig
         };

         // Optional: Prüfen, ob Action mit diesem Namen bereits existiert
         if (newActionData.name) {
             const existingAction = props.modelValue.find(a => a.name?.trim() === newActionData.name?.trim() && a.type === type);
             if (existingAction) {
                 const confirmAddAnyway = confirm(`An ${type} action named "${newActionData.name}" already exists. Do you want to add it again from the template?`);
                 if (!confirmAddAnyway) {
                     return; // Abbruch durch Benutzer
                 }
             }
         }

         // === KORRIGIERT ===
         // Arbeite mit einer Kopie des FLACHEN modelValue Arrays
         const newActions = cloneDeep(props.modelValue);
         // Füge das neue Action-Objekt zum FLACHEN Array hinzu
         newActions.push(newActionData);
         // Emittiere das aktualisierte FLATE Array (emitUpdate wandelt es um)
         emitUpdate(newActions);
         // ==================


     } catch (error) {
         console.error("Error loading action template:", error);
         alert(`Could not load the selected template: ${error.message}`);
     } finally {
         // Setze das Template Select Feld nach Auswahl oder Fehler zurück
         selectedTemplate.value = null;
     }
}

// Hauptfunktion zum Hinzufügen (wird vom Add Button aufgerufen)
function handleAddAction() {
    if (!props.isEnabled || isLoadingData.value || !selectedActionType.value) return;

    if (selectedTemplate.value) {
        // Der Watcher für selectedTemplate ruft addActionFromTemplate auf
        // Wir müssen hier nichts weiter tun, ausser sicherstellen, dass selectedTemplate gesetzt ist
        // (was es ist, da der Button disabled ist, wenn kein Template gewählt)
        // Die Logik ist im Watcher.
         console.log("Add button clicked with template selected. Watcher will handle.");

    } else {
        // Kein Template ausgewählt, füge leeres Action hinzu
        console.log(`Add button clicked, adding empty ${selectedActionType.value}`);
        addEmptyAction(selectedActionType.value);
    }
}

// --- Delete / Save / Delete Template Logik pro Action ---

function removeAction(index) {
    if (!props.isEnabled || index === undefined || index === null || index < 0 || index >= props.modelValue.length) return;

    // === KORRIGIERT ===
    // Arbeite mit einer Kopie des FLACHEN modelValue Arrays
    const newActions = cloneDeep(props.modelValue);
    // Entferne das Element aus dem FLACHEN Array
    newActions.splice(index, 1);
    // Emittiere das aktualisierte FLATE Array (emitUpdate wandelt es um)
    emitUpdate(newActions);
    // ==================
}

async function saveActionAsTemplate(action, index) {
    if (!props.isEnabled || !action || !action.name?.trim() || !action.type) {
        alert(`Action needs a name and a valid type (${action?.type}) to be saved as a template.`);
        return;
    }

    const templateType = action.type; // templateType ist 'attackRoll', 'savingThrow', 'other'
    // Erstelle ID (ähnlich wie bei Traits, muss mit Backend übereinstimmen)
    let templateId = action.name.toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/^_+|_+$/g, '');
    // Optional: Füge weitere eindeutige Teile zur ID hinzu, falls nötig (z.B. für Saving Throw: Stat+DC?)

    // Daten für POST (sollte die Action-Struktur für diesen Typ sein, ohne frontend-IDs/Properties)
    // Wir senden das Action-Objekt, wie es ist, da es die nötige Struktur haben sollte
    const templateData = cloneDeep(action);
    delete templateData.type; // Der Typ ist Teil des Endpunkts
    delete templateData.id; // Template-ID wird vom Backend generiert

    try {
        // Nutze den typ-spezifischen Endpunkt
        const response = await fetch(`http://localhost:8080/api/templates/${templateType}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(templateData)
        });
        if (response.status === 409) {
            alert(`Error: A template with the generated ID "${templateId}" already exists for type "${templateType}" (Name: "${action.name}").`);
        } else if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
        } else {
             const savedTemplate = await response.json(); // Backend sollte gespeichertes Template inkl. ID zurückgeben
             alert(`Template "${savedTemplate.name}" (ID: ${savedTemplate.id}) saved successfully for type "${templateType}"!`);
             // Lade Template-Liste für den aktuellen ActionType neu, falls nötig
             if (selectedActionType.value === templateType) {
                  actionTemplates.value = await fetchActionTemplates(templateType);
             }
        }
    } catch (error) {
         console.error(`Error saving ${templateType} template:`, error);
         alert(`Error saving template: ${error.message}`);
    }
}

async function deleteActionTemplate(templateId) {
    if (!props.isEnabled || !templateId || !selectedActionType.value) {
         alert("Cannot delete template: No template selected, no type selected, or disabled.");
         return;
    }
    const templateType = selectedActionType.value; // Lösche immer Templates des aktuell ausgewählten Typs

    // Finde den Namen für die Bestätigung
    const templateToDelete = actionTemplates.value.find(t => t.id === templateId);
    const templateName = templateToDelete ? `"${templateToDelete.name}" (ID: ${templateId})` : `ID: ${templateId}`;

    if (!confirm(`Are you sure you want to delete the template ${templateName} of type "${templateType}"? This cannot be undone.`)) {
        return;
    }

     try {
         // Nutze den typ-spezifischen Endpunkt
        const response = await fetch(`http://localhost:8080/api/templates/${templateType}/${templateId}`, {
            method: 'DELETE'
        });
         if (!response.ok) {
             const errorData = await response.json().catch(()=>({}));
            throw new Error(errorData.error || `HTTP error ${response.status}`);
         }
         alert(`Template ${templateName} of type "${templateType}" deleted successfully!`);
         // Lade Template-Liste für den aktuellen ActionType neu
         actionTemplates.value = await fetchActionTemplates(templateType);

     } catch (error) {
          console.error(`Error deleting ${templateType} template:`, error);
          alert(`Error deleting template: ${error.message}`);
     }
}

// Funktion zum Holen der Template Liste für einen Typ
async function fetchActionTemplates(type) {
    if (!type) return [];
    try {
        const response = await fetch(`http://localhost:8080/api/templates/${type}`);
        if (!response.ok) throw new Error('Failed to fetch templates');
        return await response.json(); // Sollte ein Array von { id, name } zurückgeben
    } catch (error) {
        console.error(`Error fetching ${type} templates:`, error);
        return [];
    }
}

// --- Lifecycle Hook ---
onMounted(async () => {
    isLoadingData.value = true;
    try {
        // Lade alle benötigten DnD Daten parallel
        const [ranges, dice, damage, recharge, reset, stats] = await Promise.all([
            loadDnDData('attackRanges.json'),
            loadDnDData('dice.json'),
            loadDnDData('damageTypes.json'),
            loadDnDData('recharge.json'),
            loadDnDData('resetTypes.json'),
            loadDnDData('stats.json')
        ]);
        attackRanges.value = ranges || [];
        diceOptions.value = dice || { diceMapping: {}, dieLookup: {} };
        damageTypes.value = damage || [];
        rechargeOptions.value = recharge || [];
        resetTypes.value = reset || [];
         // Stats sind ein Objekt { STR:{}, ... }, brauche nur die Keys als Array
         statsOptions.value = stats ? Object.keys(stats) : [];

    } catch (error) {
         console.error("Error loading initial data for ActionConfig:", error);
         // Setze alle Optionen auf leere Werte bei Fehler
         attackRanges.value = []; diceOptions.value = { diceMapping: {}, dieLookup: {} }; damageTypes.value = [];
         rechargeOptions.value = []; resetTypes.value = []; statsOptions.value = [];
    } finally {
        isLoadingData.value = false;
    }

    // Lade initiale Templates für den Standard-Typ ('attackRoll')
    // Wird bereits durch den immediate Watcher auf selectedActionType ausgelöst
});

// --- Lokale Refs & Logik für Uses/Recharge Disabled State (im Template verwendet) ---
// Da es in ExpansionPanels ist, brauchen wir für jedes Panel eine Logik
// Diese kann als Funktion im Template aufgerufen werden
function isRechargeDisabled(action) {
     // Recharge ist disabled, wenn Uses oder Reset Type gesetzt sind
    return (action.limitedUse?.count !== null && action.limitedUse?.count !== undefined && action.limitedUse.count > 0) ||
           (action.limitedUse?.rate !== null && action.limitedUse?.rate !== undefined && action.limitedUse.rate !== '');
}

function areUsesResetDisabled(action) {
    // Uses/Reset sind disabled, wenn Recharge gesetzt ist
    return action.recharge !== null && action.recharge !== undefined && action.recharge !== '';
}

// --- Lokale Refs & Logik für Override Disabled State (im Template verwendet) ---
// Saving Throw DC hat Override Logik ähnlich wie in SensesConfig
function isSaveDcOverride(action) {
    return action?.safeDC?.overrideValue !== null && action?.safeDC?.overrideValue !== undefined;
}
function toggleSaveDcOverride(actionIndex) {
     if (!props.isEnabled) return;
    const action = props.modelValue[actionIndex];
    // Stelle sicher, dass die Action vom Typ SavingThrow ist und safeDC existiert
    if (!action || action.type !== 'savingThrow') return;

    const nextState = !isSaveDcOverride(action);
    // Holen Sie den aktuellen Default-Wert, falls er für die Berechnung benötigt wird
    const currentDefaultDc = action.safeDC?.defaultValue ?? 0;

    // Wenn aktiviert: Nutze Default DC (neu berechnet basierend auf Stat) oder 0
    // Berechne den Default neu für den Fall, dass sich der Stat seit der letzten Berechnung geändert hat,
    // aber der Override aktiv war.
    const calculatedDefaultDc = calculateSaveDC(props.basicsData, action.saveStat); // Recalculate based on current stat
    const newValue = nextState ? (calculatedDefaultDc ?? 0) : null; // Wenn aktivieren -> neuer Default, sonst null

    // === KORRIGIERT ===
    // Erstelle eine Kopie des FLACHEN Arrays
    const newActions = cloneDeep(props.modelValue);
     // Stelle sicher, dass safeDC Objekt in der Kopie existiert
    if (!newActions[actionIndex].safeDC) {
        newActions[actionIndex].safeDC = { defaultValue: 0, overrideValue: null };
    }
    // Setze den neuen overrideValue
    newActions[actionIndex].safeDC.overrideValue = newValue;

    // Aktualisiere auch defaultValue in der Kopie, WENN Override aktiviert WIRD
    // Dies stellt sicher, dass der angezeigte Wert korrekt ist, wenn man toggelt
    if (nextState) {
         newActions[actionIndex].safeDC.defaultValue = calculatedDefaultDc; // Setze den Default auf den gerade berechneten Wert
    }

    // Emittiere das aktualisierte FLATE Array (emitUpdate wandelt es um)
    emitUpdate(newActions);
    // ==================
}


</script>

<template>
    <div :class="{'disabled-content': !isEnabled}">
        <div v-if="isLoadingData" class="pa-4 text-center text-disabled">Loading data...</div>
        <!-- Zeige die Meldung, wenn das Array leer ist -->
        <div v-else-if="modelValue.length === 0" class="pa-4 text-center text-disabled">
            No {{ actionGroupType === 'actions' ? 'actions' : 'bonus actions' }} added yet. Use the buttons below.
        </div>
        <v-expansion-panels v-else variant="accordion" multiple>
            <!-- Iteriere über das FLACHE Array -->
            <v-expansion-panel v-for="(action, index) in modelValue" :key="index" elevation="1">
                <v-expansion-panel-title :disabled="!isEnabled">
                     <!-- Zeige Action-Namen und Typ -->
                    <span class="flex-grow-1 mr-2">{{ action.name || `${action.type || 'Unknown'} ${actionGroupType === 'actions' ? 'Action' : 'Bonus Action'} ${index + 1}` }}</span>
                    <!-- Move Buttons -->
                    <v-btn icon="mdi-arrow-up-bold-box-outline" size="x-small" variant="text" @click.stop="moveAction(index, -1)" :disabled="index === 0 || !isEnabled" class="mr-1"/>
                    <v-btn icon="mdi-arrow-down-bold-box-outline" size="x-small" variant="text" @click.stop="moveAction(index, 1)" :disabled="index === modelValue.length - 1 || !isEnabled"/>
                </v-expansion-panel-title>
                <v-expansion-panel-text>
                    <!-- === Layouts basierend auf action.type === -->

                    <!-- === Layout Attack Roll === -->
                    <v-row dense v-if="action.type === 'attackRoll'">
                         <!-- ... (Layout Attack Roll wie zuvor) ... -->
                         <!-- Stellen Sie sicher, dass die Logik im Template die korrigierten Funktionen verwendet -->
                         <v-col cols="12" md="6">
                            <v-text-field label="Name" :model-value="action.name" @update:model-value="updateActionField(index, 'name', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled"/>
                         </v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Attack Bonus" :model-value="action.attackMod" @update:model-value="updateActionField(index, 'attackMod', parseInt($event, 10) || 0)" density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="4"></v-col>
                         <v-col cols="12" md="4">
                             <v-select label="Melee/Ranged" :items="attackRanges" :model-value="action.rangeType" @update:model-value="updateActionField(index, 'rangeType', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Reach Melee" :model-value="action.reachMelee" @update:model-value="updateActionField(index, 'reachMelee', parseInt($event, 10) || null)" suffix=" ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :min="0" :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Reach Ranged" :model-value="action.reachRanged" @update:model-value="updateActionField(index, 'reachRanged', parseInt($event, 10) || null)" suffix=" ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :min="0" :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Reach Disadvantage" :model-value="action.reachDisadvantage" @update:model-value="updateActionField(index, 'reachDisadvantage', parseInt($event, 10) || null)" suffix=" ft." density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :min="0" :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2"></v-col>
                         <template v-for="(damageEntry, damageIndex) in action.damage" :key="damageIndex">
                              <v-col cols="12" md="1" class="d-flex align-center py-0">
                                   <span class="text-caption">{{ damageIndex === 0 ? 'Hit:' : '' }}</span>
                              </v-col>
                             <v-col cols="12" md="1" class="d-flex align-center py-0">
                                  <span class="text-body-2">{{ calculateDisplayedAverageDamage(damageEntry.count, damageEntry.size, damageEntry.modifier) }}</span>
                             </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-number-input label="# Dice" :model-value="damageEntry.count" @update:model-value="updateActionField(index, `damage[${damageIndex}].count`, parseInt($event, 10) || 0)" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-select label="Die Size" :items="mappedDiceOptions" item-title="title" item-value="value" :model-value="damageEntry.size" @update:model-value="updateActionField(index, `damage[${damageIndex}].size`, $event)" density="compact" variant="outlined" :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-number-input label="Mod" :model-value="damageEntry.modifier" @update:model-value="updateActionField(index, `damage[${damageIndex}].modifier`, parseInt($event, 10) || 0)" density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="4" class="d-flex align-center py-0">
                                   <v-combobox label="Damage Type" :items="damageTypes" :model-value="damageEntry.type" @update:model-value="updateActionField(index, `damage[${damageIndex}].type`, $event)" density="compact" variant="outlined" :disabled="!isEnabled" hide-details="auto" class="flex-grow-1"/>
                                    <v-btn icon="mdi-close" size="x-small" variant="text" color="error" @click="removeNestedArrayItem(index, 'damage', damageIndex)" :disabled="!isEnabled || action.damage.length <= 1" aria-label="Remove damage" class="ml-1"/>
                              </v-col>
                         </template>
                         <v-col cols="12" offset-md="1" md="11" class="py-0">
                              <v-btn block color="secondary" variant="text" @click="addNestedArrayItem(index, 'damage', { count: 1, size: 6, modifier: 0, type: '' })" :disabled="!isEnabled" prepend-icon="mdi-plus-circle-outline">Add Damage Entry</v-btn>
                         </v-col>
                         <v-col cols="12">
                              <v-textarea label="Notes" :model-value="action.notes" @update:model-value="updateActionField(index, 'notes', $event)" density="compact" variant="outlined" clearable auto-grow rows="2" :disabled="!isEnabled"/>
                         </v-col>
                    </v-row>

                    <!-- === Layout Saving Throw === -->
                    <v-row dense v-else-if="action.type === 'savingThrow'">
                         <!-- ... (Layout Saving Throw wie zuvor) ... -->
                         <v-col cols="12" md="4">
                             <v-text-field label="Name" :model-value="action.name" @update:model-value="updateActionField(index, 'name', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled"/>
                         </v-col>
                         <v-col cols="12" md="2">
                             <v-select label="Save Stat" :items="statsOptions" :model-value="action.saveStat" @update:model-value="updateActionField(index, 'saveStat', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2">
                              <!-- Save DC mit Override Logik -->
                              <v-number-input
                                  label="Save DC"
                                  :model-value="isSaveDcOverride(action) ? action.safeDC?.overrideValue : action.safeDC?.defaultValue"
                                  @update:model-value="updateActionField(index, 'safeDC.overrideValue', $event === '' || $event === null ? null : parseInt($event, 10))"
                                  density="compact" variant="outlined"
                                  :readonly="!isSaveDcOverride(action)"
                                  :class="{'input-is-default': !isSaveDcOverride(action)}"
                                  control-variant="stacked" :reverse="false" inset :min="0"
                                  class="left-aligned-input"
                                  :disabled="!isEnabled"
                                   hide-details="auto"
                              >
                                 <template v-slot:append-inner>
                                     <v-tooltip location="top" :text="isSaveDcOverride(action) ? 'Use Default' : 'Override DC'">
                                         <template v-slot:activator="{ props: tooltipProps }">
                                             <v-btn v-bind="tooltipProps" :icon="isSaveDcOverride(action) ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'" variant="text" size="x-small" @click.stop="toggleSaveDcOverride(index)" :disabled="!isEnabled"/>
                                         </template>
                                     </v-tooltip>
                                 </template>
                              </v-number-input>
                         </v-col>
                         <v-col cols="12" md="4"></v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Uses" :model-value="action.limitedUse?.count" @update:model-value="updateActionField(index, 'limitedUse.count', parseInt($event, 10) || null)" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled || areUsesResetDisabled(action)" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="4">
                             <v-select label="Reset Type" :items="resetTypes" :model-value="action.limitedUse?.rate" @update:model-value="updateActionField(index, 'limitedUse.rate', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled || areUsesResetDisabled(action)" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="4">
                              <v-select label="Recharge" :items="rechargeOptions" :model-value="action.recharge" @update:model-value="updateActionField(index, 'recharge', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled || isRechargeDisabled(action)" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2"></v-col>
                         <template v-for="(damageEntry, damageIndex) in action.damage" :key="damageIndex">
                              <v-col cols="12" md="1" class="d-flex align-center py-0">
                                   <span class="text-caption">{{ damageIndex === 0 ? 'Fail:' : '' }}</span>
                              </v-col>
                              <v-col cols="12" md="1" class="d-flex align-center py-0">
                                   <span class="text-body-2">{{ calculateDisplayedAverageDamage(damageEntry.count, damageEntry.size, damageEntry.modifier) }}</span>
                              </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-number-input label="# Dice" :model-value="damageEntry.count" @update:model-value="updateActionField(index, `damage[${damageIndex}].count`, parseInt($event, 10) || 0)" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-select label="Die Size" :items="mappedDiceOptions" item-title="title" item-value="value" :model-value="damageEntry.size" @update:model-value="updateActionField(index, `damage[${damageIndex}].size`, $event)" density="compact" variant="outlined" :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="2" class="py-0">
                                   <v-number-input label="Mod" :model-value="damageEntry.modifier" @update:model-value="updateActionField(index, `damage[${damageIndex}].modifier`, parseInt($event, 10) || 0)" density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled" hide-details="auto"/>
                              </v-col>
                              <v-col cols="12" md="4" class="d-flex align-center py-0">
                                   <v-combobox label="Damage Type" :items="damageTypes" :model-value="damageEntry.type" @update:model-value="updateActionField(index, `damage[${damageIndex}].type`, $event)" density="compact" variant="outlined" :disabled="!isEnabled" hide-details="auto" class="flex-grow-1"/>
                                    <v-btn icon="mdi-close" size="x-small" variant="text" color="error" @click="removeNestedArrayItem(index, 'damage', damageIndex)" :disabled="!isEnabled || action.damage.length <= 1" aria-label="Remove damage" class="ml-1"/>
                              </v-col>
                         </template>
                         <v-col cols="12" offset-md="1" md="11" class="py-0">
                              <v-btn block color="secondary" variant="text" @click="addNestedArrayItem(index, 'damage', { count: 1, size: 6, modifier: 0, type: '' })" :disabled="!isEnabled" prepend-icon="mdi-plus-circle-outline">Add Damage Entry (Fail)</v-btn>
                         </v-col>
                         <v-col cols="12">
                              <v-textarea label="Notes" :model-value="action.notes" @update:model-value="updateActionField(index, 'notes', $event)" density="compact" variant="outlined" clearable auto-grow rows="2" :disabled="!isEnabled"/>
                         </v-col>
                          <v-col cols="12" md="1" class="d-flex align-center">
                             <span class="text-caption">Success:</span>
                         </v-col>
                          <v-col cols="12" md="11">
                              <v-textarea label="Success Description" :model-value="action.successNotes" @update:model-value="updateActionField(index, 'successNotes', $event)" density="compact" variant="outlined" clearable auto-grow rows="2" :disabled="!isEnabled"/>
                         </v-col>
                          <v-col cols="12" md="2" class="d-flex align-center">
                             <span class="text-caption">Success or Fail:</span>
                         </v-col>
                          <v-col cols="12" md="10">
                              <v-textarea label="Success or Fail Description" :model-value="action.succesOrFailNotes" @update:model-value="updateActionField(index, 'succesOrFailNotes', $event)" density="compact" variant="outlined" clearable auto-grow rows="2" :disabled="!isEnabled"/>
                         </v-col>
                    </v-row>

                    <!-- === Layout Other === -->
                    <v-row dense v-else-if="action.type === 'other'">
                         <!-- ... (Layout Other wie zuvor) ... -->
                         <v-col cols="12" md="6">
                            <v-text-field label="Name" :model-value="action.name" @update:model-value="updateActionField(index, 'name', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled"/>
                         </v-col>
                         <v-col cols="12" md="6"></v-col>
                         <v-col cols="12" md="2">
                             <v-number-input label="Uses" :model-value="action.limitedUse?.count" @update:model-value="updateActionField(index, 'limitedUse.count', parseInt($event, 10) || null)" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset :disabled="!isEnabled || areUsesResetDisabled(action)" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="4">
                             <v-select label="Reset Type" :items="resetTypes" :model-value="action.limitedUse?.rate" @update:model-value="updateActionField(index, 'limitedUse.rate', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled || areUsesResetDisabled(action)" hide-details="auto"/>
                         </v-col>
                          <v-col cols="12" md="4">
                              <v-select label="Recharge" :items="rechargeOptions" :model-value="action.recharge" @update:model-value="updateActionField(index, 'recharge', $event)" density="compact" variant="outlined" clearable :disabled="!isEnabled || isRechargeDisabled(action)" hide-details="auto"/>
                         </v-col>
                         <v-col cols="12" md="2"></v-col>
                         <v-col cols="12">
                              <v-textarea label="Description" :model-value="action.description" @update:model-value="updateActionField(index, 'description', $event)" density="compact" variant="outlined" clearable auto-grow rows="3" :disabled="!isEnabled"/>
                         </v-col>
                    </v-row>

                    <!-- === DeleteUndTemplate Zeile (für alle Typen) === -->
                    <v-row dense class="mt-4">
                         <v-col cols="12" md="6">
                            <v-btn @click="removeAction(index)" color="error" variant="outlined" :disabled="!isEnabled" block>Delete {{ actionGroupType === 'actions' ? 'Action' : 'Bonus Action' }}</v-btn>
                        </v-col>
                         <v-col cols="12" md="3">
                             <!-- Deaktiviere Speichern, wenn Name leer oder Typ unbekannt -->
                            <v-btn @click="saveActionAsTemplate(action, index)" color="secondary" variant="outlined" :disabled="!isEnabled || !action.name?.trim() || !action.type" block>Save Template</v-btn>
                        </v-col>
                         <v-col cols="12" md="3">
                              <!-- Dropdown zum Auswählen/Löschen von Templates des AKTUELEN Action-Typs -->
                             <v-menu>
                                 <template v-slot:activator="{ props: menuProps }">
                                     <!-- Deaktiviere Button, wenn keine Templates geladen oder Typ unbekannt -->
                                      <v-btn color="secondary" variant="outlined" :disabled="!isEnabled || actionTemplates.length === 0 || !action.type" block v-bind="menuProps">Delete Template</v-btn>
                                 </template>
                                 <v-list density="compact">
                                     <v-list-item
                                         v-for="template in actionTemplates"
                                         :key="template.id"
                                         @click="deleteActionTemplate(template.id)"
                                         :title="template.name"
                                         density="compact"
                                     >
                                         <template v-slot:append><v-icon icon="mdi-delete" size="small"></v-icon></template>
                                     </v-list-item>
                                     <v-list-item v-if="actionTemplates.length === 0" disabled title="No templates available for this type"></v-list-item>
                                 </v-list>
                             </v-menu>
                         </v-col>
                    </v-row>

                </v-expansion-panel-text>
            </v-expansion-panel>
        </v-expansion-panels>

         <!-- === Add Action Bar === -->
         <v-row dense class="mt-4">
              <!-- Action Typ Toggle -->
              <v-col cols="12" md="6">
                   <v-btn-toggle
                       v-model="selectedActionType"
                       color="primary"
                       divided mandatory block
                       :disabled="!isEnabled || isLoadingData"
                       class="action-type-toggle"
                       density="compact"
                   >
                       <v-btn value="attackRoll" class="flex-grow-1">Attack Roll</v-btn>
                       <v-btn value="savingThrow" class="flex-grow-1">Saving Throw</v-btn>
                       <v-btn value="other" class="flex-grow-1">Other</v-btn>
                   </v-btn-toggle>
              </v-col>
              <!-- Load Template Select -->
              <v-col cols="12" md="4">
                  <v-select
                     label="Load Template"
                     :items="templateOptions"
                     v-model="selectedTemplate"
                     density="compact" variant="outlined"
                     clearable
                     :disabled="!isEnabled || isLoadingData || templateOptions.length === 0 || !selectedActionType"
                     hide-details="auto"
                     placeholder="Select a template"
                  >
                       <template v-slot:no-data>
                             <v-list-item v-if="isLoadingData">
                                  <v-progress-circular indeterminate color="primary" size="16" class="mr-2"></v-progress-circular>
                                 Loading templates...
                             </v-list-item>
                             <v-list-item v-else-if="!selectedActionType">
                                 Select an Action Type first.
                             </v-list-item>
                             <v-list-item v-else>
                                 No templates found for "{{ selectedActionType }}".
                             </v-list-item>
                         </template>
                  </v-select>
              </v-col>
              <!-- Add Button -->
              <v-col cols="12" md="2">
                   <v-btn
                       block
                       color="success"
                       variant="flat"
                       @click="handleAddAction"
                       :disabled="!isEnabled || isLoadingData || !selectedActionType"
                       prepend-icon="mdi-plus"
                   >
                       Add
                   </v-btn>
              </v-col>
         </v-row>
         <!-- ENDE Add Action Bar -->

    </div>
</template>

<style scoped>
.disabled-content {
    opacity: 0.6;
    pointer-events: none;
}

/* Styles für linksbündigen Number Input und Append/Prepend Platz */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
}
:deep(.left-aligned-input .v-field__append-inner),
:deep(.left-aligned-input .v-field__prepend-inner) {
    align-items: center;
    padding-inline-start: 0 !important; /* Standardmäßiges Padding entfernen */
    padding-inline-end: 0 !important;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn),
:deep(.left-aligned-input .v-field__prepend-inner .v-btn) {
    min-width: auto !important; padding: 0 4px !important; margin: 0 1px !important;
}
:deep(.left-aligned-input .v-field__suffix) {
     padding-inline-start: 4px !important; opacity: 0.8;
}

.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
}

/* Styling für den dreifach-Button (v-btn-toggle) */
.action-type-toggle :deep(.v-btn) {
    border-radius: 0 !important; /* Keine Abrundung standardmäßig */
}
.action-type-toggle :deep(.v-btn:first-child) {
     /* Linker äußerer Rand abrunden */
     border-top-left-radius: 4px !important;
     border-bottom-left-radius: 4px !important;
}
.action-type-toggle :deep(.v-btn:last-child) {
     /* Rechter äußerer Rand abrunden */
     border-top-right-radius: 4px !important;
     border-bottom-right-radius: 4px !important;
}
/* Optional: Stellt sicher, dass die Ränder zwischen den Buttons nicht doppelt dick sind */
.action-type-toggle :deep(.v-btn + .v-btn) {
    border-left: none !important;
}
</style>