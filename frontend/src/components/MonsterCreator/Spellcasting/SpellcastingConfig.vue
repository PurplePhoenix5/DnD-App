<!-- frontend/src/components/MonsterCreator/Spellcasting/SpellcastingConfig.vue -->
<script setup>
import { ref, onMounted, computed, watch } from 'vue';
import { cloneDeep, set, get } from 'lodash';
import { getAllSpells, getSpellData } from '../../../utils/spellsData.js'; // Importiere Spell-Funktionen
import { statModifier } from '../../../utils/mathRendering.js'; // Für Ability Modifier
import { VSelect } from 'vuetify/components/VSelect';
import { VNumberInput } from 'vuetify/lib/components/index.mjs';
import { VBtn } from 'vuetify/components/VBtn';
import { VIcon } from 'vuetify/components/VIcon';
import { VRow, VCol } from 'vuetify/components/VGrid';
import { VTextField } from 'vuetify/components/VTextField';
import { VSwitch } from 'vuetify/components/VSwitch';
import { VTooltip } from 'vuetify/components/VTooltip'; // Für Tooltips
import { VChip } from 'vuetify/components/VChip'; // Für die Anzeige der Spells
import { VDivider } from 'vuetify/components/VDivider'; // Optional für Trenner
import { VProgressCircular } from 'vuetify/components/VProgressCircular'; // Für Ladeanzeige

const props = defineProps({
  modelValue: { type: Object, required: true }, // Das gesamte spellcasting-Objekt
  basicsData: { type: Object, required: true }, // Brauchen Stats und PB
  isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']);

// --- Lade Spell Daten ---
// Wir brauchen das volle Objekt, um ggf. später Spell-Details zu holen,
// aber für die Combobox reicht eine Liste der Namen.
const allSpellsData = ref({}); // Das rohe Objekt von spells.json
const spellNames = ref([]); // Array von Spell-Namen für Combobox
const isLoadingSpells = ref(true);

// --- Lokale Refs für Override Toggles (DC, Bonus) ---
// Initialisiere mit dem Wert aus modelValue, falls vorhanden
const isDcOverride = ref(props.modelValue.dc?.overrideValue !== null && props.modelValue.dc?.overrideValue !== undefined);
const isBonusOverride = ref(props.modelValue.bonus?.overrideValue !== null && props.modelValue.bonus?.overrideValue !== undefined);


// --- Lokale Refs für Add Spell Input (pro Frequenz) ---
const selectedSpellToAdd = ref({
    atWill: null,
    once: null,
    twice: null,
    thrice: null,
});
const spellNotesToAdd = ref({
    atWill: '',
    once: '',
    twice: '',
    thrice: '',
});

// Frequenzen definieren (entspricht den Schlüsseln im spellcasting-Objekt)
const spellFrequencies = [
    { id: 'atWill', label: 'At Will:' },
    { id: 'once', label: '1/Day Each:' },
    { id: 'twice', label: '2/Day Each:' },
    { id: 'thrice', label: '3/Day Each:' },
];

// --- Helpers ---
function emitUpdate(key, value) {
    // Klonen Sie immer das gesamte spellcasting-Objekt, bevor Sie eine Eigenschaft setzen
    const newSpellcasting = cloneDeep(props.modelValue);

    // Stellen Sie sicher, dass verschachtelte Objekte existieren, bevor Sie setzen
    // Dies ist besonders wichtig für 'dc.overrideValue', 'bonus.overrideValue', 'limitedUse.count', etc.
    // Vuetify bindet an v-model und emittiert direkt die neuen Werte.
    // set(newSpellcasting, key, value) von lodash kann verschachtelte Pfade erstellen.
    // Aber für die Override-Toggle-Buttons, die separate Funktionen aufrufen, ist dies expliziter.
    // Die Computed Properties für DC/Bonus Overrides kümmern sich darum.
    // Für einfachere Pfade reicht set.
    set(newSpellcasting, key, value);

    emit('update:field', { key: 'spellcasting', value: newSpellcasting });
}

// --- Computed Properties ---

// Casting Stat Select Items
const castingStatOptions = computed(() => ['INT', 'WIS', 'CHA']);

// Spell Save DC
const dcOverrideValue = computed({
    get: () => props.modelValue.dc?.overrideValue,
    set: (v) => {
        // Konvertiere zu Zahl, null für leere Eingabe
        const newValue = v === '' || v === null ? null : parseInt(v, 10);
         // Stelle sicher, dass das 'dc' Objekt existiert, falls es null/undefined ist
        const currentDc = props.modelValue.dc || {};
        emitUpdate('dc', { ...currentDc, overrideValue: newValue });
        // isDcOverride wird vom Watcher aktualisiert
    }
});
const dcDefaultValue = computed(() => props.modelValue.dc?.defaultValue ?? 0); // Default 0, wird unten neu berechnet
const displayedDc = computed(() => isDcOverride.value ? dcOverrideValue.value : dcDefaultValue.value);

// Spell Attack Bonus
const bonusOverrideValue = computed({
    get: () => props.modelValue.bonus?.overrideValue,
    set: (v) => {
        const newValue = v === '' || v === null ? null : parseInt(v, 10);
         // Stelle sicher, dass das 'bonus' Objekt existiert
        const currentBonus = props.modelValue.bonus || {};
        emitUpdate('bonus', { ...currentBonus, overrideValue: newValue });
        // isBonusOverride wird vom Watcher aktualisiert
    }
});
const bonusDefaultValue = computed(() => props.modelValue.bonus?.defaultValue ?? 0); // Default 0, wird unten neu berechnet
const displayedBonus = computed(() => isBonusOverride.value ? bonusOverrideValue.value : bonusDefaultValue.value);


// Recalculate Default DC/Bonus
function recalculateSpellDefaults() {
    console.log("SpellcastingConfig: Attempting to recalculate spell defaults...");
    // Stelle sicher, dass basicsData, stats, PB und castingStat verfügbar sind
    const stats = props.basicsData?.stats;
    const pb = props.basicsData?.PB; // Kann 0 sein, aber sollte number sein
    const castingStat = props.modelValue?.stat; // Casting stat from modelValue

    // Prüfe, ob wir genug Daten für die Berechnung haben
    if (!stats || typeof pb !== 'number' || !castingStat || stats[castingStat] === undefined) {
        console.warn("SpellcastingConfig: Cannot calculate spell defaults due to missing required data.", { stats: stats, PB: pb, castingStat: castingStat, modelValue: props.modelValue });
        return; // Abbrechen, wenn die Daten nicht vorhanden sind
    }
    // ===================================================

    console.log(`SpellcastingConfig: Calculating defaults for Stat: ${castingStat}, PB: ${pb}, Score: ${stats[castingStat]}`);

    const modifier = statModifier(stats[castingStat]);

    const newDefaultDc = 8 + pb + modifier;
    const newDefaultBonus = pb + modifier;

    // Check if defaults need updating AND overrides are not active
    const currentDc = props.modelValue.dc || {};
    const currentBonus = props.modelValue.bonus || {};

    const defaultsNeedUpdate =
        (currentDc.overrideValue === null && currentDc.defaultValue !== newDefaultDc) ||
        (currentBonus.overrideValue === null && currentBonus.defaultValue !== newDefaultBonus);


    if (defaultsNeedUpdate) {
        console.log(`SpellcastingConfig: Updating defaults from ${currentDc.defaultValue}/${currentBonus.defaultValue} to ${newDefaultDc}/${newDefaultBonus}`);
        // Prepare updated dc and bonus objects - only update defaultValue if override is null
        const updatedDc = currentDc.overrideValue === null ? { ...currentDc, defaultValue: newDefaultDc } : currentDc;
        const updatedBonus = currentBonus.overrideValue === null ? { ...currentBonus, defaultValue: newDefaultBonus } : currentBonus;

        // Emittiere das gesamte spellcasting-Objekt mit aktualisierten Defaults
        // Nur wenn sich etwas geändert hat und Overrides nicht aktiv sind
        emit('update:field', {
            key: 'spellcasting',
            value: {
                ...props.modelValue, 
                dc: updatedDc,
                bonus: updatedBonus
                // Behalte alle anderen Properties bei
            }
        });
    } else {
        console.log("SpellcastingConfig: Defaults are up-to-date or override is active. No emission needed.");
    }
}

// Watcher für Default DC/Bonus Neuberechnung
watch([
    () => props.basicsData?.stats, // Watching the stats object itself
    () => props.basicsData?.PB,      // Watching PB
    () => props.modelValue?.stat   // Watching the casting stat string
], (newValues, oldValues) => {
     console.log("SpellcastingConfig: Watcher triggered for defaults.", { newValues, oldValues });
    recalculateSpellDefaults(); // VERSUCHE, neu zu berechnen
}, { deep: true, immediate: true }); // immediate: true, um beim Laden initiale Werte zu setzen


// Watcher für Override State Refs (aktualisieren sich, wenn modelValue sich ändert)
// Diese Watcher stellen sicher, dass die lokalen isXOverride Refs mit dem modelValue synchron bleiben
// z.B., wenn das Monster geladen wird oder der Override extern geändert wird
watch(() => props.modelValue.dc?.overrideValue, (newVal) => { isDcOverride.value = newVal !== null && newVal !== undefined; }, { immediate: true });
watch(() => props.modelValue.bonus?.overrideValue, (newVal) => { isBonusOverride.value = newVal !== null && newVal !== undefined; }, { immediate: true });


// --- Toggle Methoden für Override ---
function toggleDcOverride() {
    if (!props.isEnabled) return;
    const nextState = !isDcOverride.value;
    // Setze overrideValue: Default, wenn aktiviert; null, wenn deaktiviert
    const newValue = nextState ? (dcDefaultValue.value ?? 0) : null; // Nutze default Wert beim Aktivieren
    // Aktualisiere direkt über emitUpdate, das den Watcher triggert und isDcOverride aktualisiert
    // Stellen Sie sicher, dass das DC-Objekt existiert
    const currentDc = props.modelValue.dc || {};
    emitUpdate('dc', { ...currentDc, overrideValue: newValue });
}

function toggleBonusOverride() {
    if (!props.isEnabled) return;
    const nextState = !isBonusOverride.value;
    const newValue = nextState ? (bonusDefaultValue.value ?? 0) : null; // Nutze default Wert beim Aktivieren
     // Stellen Sie sicher, dass das Bonus-Objekt existiert
    const currentBonus = props.modelValue.bonus || {};
    emitUpdate('bonus', { ...currentBonus, overrideValue: newValue });
}


// --- Add Spell Logik ---
async function handleAddSpell(frequency) {
    // Prüfe auf isEnabled, Ladezustand, ausgewählten Spell
    if (!props.isEnabled || isLoadingSpells.value || !selectedSpellToAdd.value[frequency]) {
        return;
    }

    const spellName = selectedSpellToAdd.value[frequency];
    const notes = spellNotesToAdd.value[frequency] || ''; // Sicherstellen, dass Note immer ein String ist

    // Optional: Prüfen, ob der Spell bereits in dieser Frequenzliste ist
    const currentList = props.modelValue[frequency] || [];
    if (currentList.some(entry => entry.name === spellName)) {
        alert(`Spell "${spellName}" is already in the "${spellFrequencies.find(f => f.id === frequency).label.replace(':', '').trim()}" list.`);
        return;
    }

    // Erstelle neuen Spell-Eintrag (entsprechend dem JSON-Schema)
    const newSpellEntry = {
        name: spellName,
        note: notes
    };

    // Füge neuen Eintrag hinzu und emittiere das Update für das gesamte spellcasting-Objekt
    // Kopiere zuerst das gesamte Objekt
    const newSpellcasting = cloneDeep(props.modelValue);
    // Stelle sicher, dass das Array für die Frequenz existiert
    if (!Array.isArray(newSpellcasting[frequency])) {
         newSpellcasting[frequency] = [];
    }
    newSpellcasting[frequency].push(newSpellEntry);

    emit('update:field', { key: 'spellcasting', value: newSpellcasting });


    // Felder nach dem Hinzufügen zurücksetzen
    selectedSpellToAdd.value[frequency] = null;
    spellNotesToAdd.value[frequency] = '';
}

// --- Remove Spell Logik ---
function handleRemoveSpell(frequency, index) {
    if (!props.isEnabled) return;

    const currentList = props.modelValue[frequency] || [];
    if (index < 0 || index >= currentList.length) return; // Index-Check

     // Kopiere das gesamte spellcasting-Objekt
     const newSpellcasting = cloneDeep(props.modelValue);
     // Stelle sicher, dass das Array existiert
     if (!Array.isArray(newSpellcasting[frequency])) {
          console.warn(`Attempted to remove spell from non-array frequency: ${frequency}`);
          return;
     }
    // Entferne das Element
    newSpellcasting[frequency].splice(index, 1);

    emit('update:field', { key: 'spellcasting', value: newSpellcasting });
}


// --- Lifecycle Hook ---
onMounted(async () => {
    isLoadingSpells.value = true;
    try {
        const spells = await getAllSpells(); // Nutze getAllSpells aus dem Cache-Modul
        if (spells) {
            allSpellsData.value = spells;
            spellNames.value = Object.keys(spells).sort();
        } else {
            console.error("Failed to load spell data on mount.");
            spellNames.value = [];
        }
    } catch (error) {
        console.error("Error fetching spell data:", error);
        spellNames.value = [];
    } finally {
        isLoadingSpells.value = false;
    }
});

</script>

<template>
    <div :class="{'disabled-content': !isEnabled}">
        <div v-if="isLoadingSpells" class="pa-4 text-center text-disabled">
             <v-progress-circular indeterminate color="primary" size="24"></v-progress-circular>
            Loading spell data...
        </div>
        <v-row v-else dense>
            <!-- Zeile 1: Generelle Spellcasting-Einstellungen -->
            <v-col cols="12" md="2">
                <v-select
                    label="Casting Stat"
                    :items="castingStatOptions"
                    :model-value="modelValue.stat"
                    @update:model-value="emitUpdate('stat', $event)"
                    density="compact" variant="outlined"
                    clearable
                    :disabled="!isEnabled"
                    hide-details="auto"
                />
            </v-col>
             <v-col cols="12" md="2">
                <!-- Spell Save DC -->
                <v-number-input
                    label="Spell Save DC"
                    :model-value="displayedDc"
                    :min="0"
                    @update:model-value="dcOverrideValue = $event"
                    density="compact" variant="outlined"
                    :readonly="!isDcOverride"
                    :class="{'input-is-default': !isDcOverride}"
                    control-variant="stacked" :reverse="false" inset
                    class="left-aligned-input"
                    :disabled="!isEnabled"
                     hide-details="auto"
                >
                   <template v-slot:append-inner>
                       <v-tooltip location="top" :text="isDcOverride ? 'Use Default' : 'Override DC'">
                           <template v-slot:activator="{ props: tooltipProps }">
                               <v-btn v-bind="tooltipProps" :icon="isDcOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'" variant="text" size="x-small" @click.stop="toggleDcOverride" :disabled="!isEnabled"/>
                           </template>
                       </v-tooltip>
                   </template>
                </v-number-input>
            </v-col>
            <v-col cols="12" md="2">
                <!-- Spell Attack Bonus -->
                <v-number-input
                    label="Spell Attack Bonus"
                    :model-value="displayedBonus"
                    :min="0"
                    @update:model-value="bonusOverrideValue = $event"
                    density="compact" variant="outlined"
                    :readonly="!isBonusOverride"
                    :class="{'input-is-default': !isBonusOverride}"
                    control-variant="stacked" :reverse="false" inset
                    class="left-aligned-input"
                    :disabled="!isEnabled"
                    hide-details="auto"
                >
                   <template v-slot:append-inner>
                       <v-tooltip location="top" :text="isBonusOverride ? 'Use Default' : 'Override Bonus'">
                           <template v-slot:activator="{ props: tooltipProps }">
                               <v-btn v-bind="tooltipProps" :icon="isBonusOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'" variant="text" size="x-small" @click.stop="toggleBonusOverride" :disabled="!isEnabled"/>
                           </template>
                       </v-tooltip>
                   </template>
                </v-number-input>
            </v-col>
            <v-col cols="12" md="1" class="d-flex align-center justify-center"> <!-- Zentriere Icons/Switches -->
                <v-tooltip location="top" text="Uses spells with Attack Rolls">
                    <template v-slot:activator="{ props: tooltipProps }">
                         <div class="d-flex align-center" v-bind="tooltipProps"> <!-- Flex Container für Icon und Switch -->
                              <v-icon icon="mdi-sword" size="small" class="mr-1"></v-icon> <!-- Attack Icon -->
                              <v-switch
                                  label="" 
                                  inset density="compact" color="primary"
                                  :model-value="modelValue.hasAttackrolls"
                                  @update:model-value="emitUpdate('hasAttackrolls', $event)"
                                  :disabled="!isEnabled"
                                  hide-details
                              />
                         </div>
                    </template>
                </v-tooltip>
            </v-col>
            <v-col cols="12" md="1" class="d-flex align-center justify-center"> <!-- Zentriere Icons/Switches -->
                <v-tooltip location="top" text="Requires Material Components">
                    <template v-slot:activator="{ props: tooltipProps }">
                         <div class="d-flex align-center" v-bind="tooltipProps"> <!-- Flex Container für Icon und Switch -->
                             <v-icon icon="mdi-flask-outline" size="small" class="mr-1"></v-icon> <!-- Components Icon -->
                             <v-switch
                                 label="" 
                                 inset density="compact" color="primary"
                                 :model-value="modelValue.requiresSComponents"
                                 @update:model-value="emitUpdate('requiresSComponents', $event)"
                                 :disabled="!isEnabled"
                                 hide-details
                             />
                         </div>
                    </template>
                </v-tooltip>
            </v-col>
            <!-- 4 breit leer lassen -->
            <v-col cols="12" md="4">
                 <!-- Placeholder oder zukünftige Elemente -->
            </v-col>

            <v-divider class="my-4"></v-divider>

            <!-- Spell Lists Section -->
            <!-- Iteriere über die Frequenzen -->
            <template v-for="freq in spellFrequencies" :key="freq.id">
                 <!-- Zeile zum Hinzufügen von Spells -->
                 <v-col cols="12" md="2" class="d-flex align-center py-0">
                    <span class="text-subtitle-2">{{ freq.label }}</span>
                 </v-col>
                 <v-col cols="12" md="4" class="py-0">
                     <v-combobox
                        label="Search Spell"
                        :items="spellNames"
                        v-model="selectedSpellToAdd[freq.id]"
                        density="compact" variant="outlined"
                        clearable
                        :disabled="!isEnabled || isLoadingSpells || spellNames.length === 0"
                        hide-details="auto"
                        placeholder="Type or select spell"
                        @focus="getAllSpells"
                     >
                         <template v-slot:no-data>
                             <v-list-item v-if="isLoadingSpells">
                                  <v-progress-circular indeterminate color="primary" size="16" class="mr-2"></v-progress-circular>
                                 Loading spells...
                             </v-list-item>
                             <v-list-item v-else>
                                 No spells found matching your search.
                             </v-list-item>
                         </template>
                     </v-combobox>
                 </v-col>
                 <v-col cols="12" md="5" class="py-0">
                    <v-tooltip location="top" text="z.B. 'included in AC' oder 'level 4 version'">
                        <template v-slot:activator="{ props: tooltipProps }">
                            <v-text-field
                                v-bind="tooltipProps"
                                label="Add Notes (for this entry)"
                                v-model="spellNotesToAdd[freq.id]"
                                density="compact" variant="outlined"
                                clearable
                                :disabled="!isEnabled || !selectedSpellToAdd[freq.id]"
                                hide-details="auto"
                            />
                        </template>
                    </v-tooltip>
                 </v-col>
                 <v-col cols="12" md="1" class="d-flex align-center py-0">
                      <v-tooltip location="top" text="Add Spell">
                          <template v-slot:activator="{ props: tooltipProps }">
                              <v-btn
                                  v-bind="tooltipProps"
                                  icon="mdi-plus"
                                  color="success"
                                  variant="text"
                                  @click="handleAddSpell(freq.id)"
                                  :disabled="!isEnabled || !selectedSpellToAdd[freq.id]"
                                  aria-label="Add spell"
                              />
                          </template>
                      </v-tooltip>
                 </v-col>

                 <!-- Zeile zur Anzeige der hinzugefügten Spells -->
                 <!-- Lässt 2 Spalten vorne leer -->
                 <v-col cols="12" offset-md="2" md="10" class="py-0">
                    <!-- Überprüfe, ob die Liste für diese Frequenz existiert und nicht leer ist -->
                    <template v-if="modelValue[freq.id]?.length > 0">
                         <div class="d-flex flex-wrap ga-1 mt-2"> <!-- ga-1 für Abstand zwischen Chips -->
                            <v-chip
                                v-for="(spellEntry, spellIndex) in modelValue[freq.id]"
                                :key="`${freq.id}-${spellIndex}-${spellEntry.name}`"
                                closable
                                @click:close="handleRemoveSpell(freq.id, spellIndex)"
                                :disabled="!isEnabled"
                                size="large"
                                color="primary"
                                variant="outlined"
                            >
                                <div>
                                     {{ spellEntry.name }}
                                     <div v-if="spellEntry.note" class="text-caption">{{ spellEntry.note }}</div>
                                </div>
                             </v-chip>
                         </div>
                    </template>
                    <div v-else class="text-disabled text-caption mt-2">No spells added for this frequency.</div>
                 </v-col>
                  <!-- Trenner zwischen Frequenzen, außer nach der letzten -->
                  <v-col cols="12" v-if="freq.id !== 'thrice'">
                     <v-divider class="my-2"></v-divider>
                  </v-col>
            </template>

        </v-row>
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
:deep(.left-aligned-input .v-field__append-inner) {
    align-items: center;
    padding-inline-start: 0 !important;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important; padding: 0 4px !important; margin: 0 1px !important;
}
.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
}

/* Styling für Switch Labels, um sie näher an die Switches zu bringen */
/* Labels wurden jetzt entfernt, dieser Stil wird ggf. nicht mehr benötigt, */
/* aber .v-switch container kann nützlich sein */
.v-switch label { /* Dieser Stil könnte entfernt werden, wenn Labels weg sind */
    margin-inline-start: 0px !important; /* Wichtiger: Platz für Icon davor schaffen */
}

/* Neues Styling für den Flex-Container um Icon und Switch */
.v-col.d-flex.align-center > .d-flex.align-center {
    /* Stellt sicher, dass der innere flex container die verfügbare Breite nutzt, falls nötig */
    width: 100%;
    /* Zentrierung wurde zur Spalte hinzugefügt, aber hier könnte man weiter justieren */
}

</style>