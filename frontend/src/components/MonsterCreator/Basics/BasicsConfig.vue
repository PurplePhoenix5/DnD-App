<!-- frontend/src/components/MonsterCreator/Basics/BasicsConfig.vue -->
<script setup>
import { computed, ref, onMounted, watch } from 'vue';
import { cloneDeep, set } from 'lodash';
import { loadDnDData } from '../../../utils/dndDataService.js'; 
import { statModifier, calculateInitiativeBonus, renderBonus } from '../../../utils/mathRendering.js'; 

const props = defineProps({
  modelValue: { type: Object, required: true }, // Erhält das 'basics'-Objekt
});
const emit = defineEmits(['update:modelValue']);

// --- Lade Daten für Selects ---
const sizes = ref([]);
const diceTypes = ref([]); // Für HD Size Dropdown
const types = ref([]);
const alignments = ref([]);
const languagesList = ref([]); // Liste der reinen Sprachnamen
const acTypes = ref([]);
const crDataList = ref([]); 

onMounted(async () => {
    // Lade alle benötigten Daten parallel
    const [sizeData, diceData, typeData, alignmentData, langData, acTypeData, crData] = await Promise.all([
        loadDnDData('sizes.json'),
        loadDnDData('dice.json'),
        loadDnDData('monsterTypes.json'),
        loadDnDData('alignments.json'),
        loadDnDData('languages.json'),
        loadDnDData('acTypes.json'),
        loadDnDData('crData.json') // Lade CR Daten hier
    ]);

    if (sizeData) sizes.value = sizeData.creatureSizes || [];
    if (diceData) diceTypes.value = Object.keys(diceData.diceMapping || {}).map(d => ({ title: d, value: diceData.diceMapping[d] }));
    if (typeData) types.value = typeData || [];
    if (alignmentData) alignments.value = alignmentData || [];
    if (langData) languagesList.value = langData.map(l => l.name);
    if (acTypeData) acTypes.value = acTypeData || [];
    if (crData) crDataList.value = crData || []; // Speichere geladene CR Daten
});

// --- Lokale Refs für Override Toggles ---
const isHDOverride = ref(props.modelValue.HP?.overrideDie !== null);
const isInitOverride = ref(props.modelValue.Initiative?.initOverrideValue !== null);

// Hilfsfunktion zum Updaten des modelValue Objekts
function updateValue(key, value) {
    // Erstelle immer eine tiefe Kopie, um das Original nicht zu mutieren
    const newObject = cloneDeep(props.modelValue);
    // Verwende lodash.set, um den Wert am Pfad zu setzen
    set(newObject, key, value);
    // Sende das *gesamte* aktualisierte Objekt
    emit('update:modelValue', newObject);
}


// --- Computed Properties mit Getter/Setter für v-model ---
const name = computed({
  get: () => props.modelValue.name,
  set: (value) => updateValue('name', value)
});
const cr = computed({
  get: () => props.modelValue.CR, // Liest den numerischen CR-Wert
  set: (value) => {
    // value kommt vom v-select und sollte bereits der numerische Wert (numeric) sein
    const numericCR = typeof value === 'number' ? value : null;
    updateValue('CR', numericCR);
  }
});
const size = computed({
  get: () => props.modelValue.size,
  set: (value) => {
      updateValue('size', value);
      // Setze Default HD Size, WENN kein Override aktiv ist
      if (!isHDOverride.value) { // Prüfe lokalen Toggle-State
           const hdMapping = { 'Tiny': 4, 'Small': 6, 'Medium': 8, 'Large': 10, 'Huge': 12, 'Gargantuan': 20, 'Titan': 20 };
           const defaultDie = hdMapping[value] || 8;
           // Update direkt über den Pfad
           updateValue('HP.defaultDie', defaultDie);
           // Stelle sicher, dass overrideDie null bleibt
           updateValue('HP.overrideDie', null);
       }
  }
});
const hpHD = computed({
  get: () => props.modelValue.HP?.HDAmount,
  set: (value) => updateValue('HP.HDAmount', Math.max(1, parseInt(value, 10) || 1)) // Min 1 sicherstellen
});
const hpTypeDefault = computed(() => props.modelValue.HP?.defaultDie ?? 8);
const hpTypeOverride = computed({
  get: () => props.modelValue.HP?.overrideDie,
  set: (value) => {
      const newOverride = value ? parseInt(value, 10) : null;
      updateValue('HP.overrideDie', newOverride);
      // Aktualisiere den Toggle-State, falls der Wert manuell gelöscht wird
      isHDOverride.value = newOverride !== null;
  }
});
const hpModifier = computed({
  get: () => props.modelValue.HP?.HPmodifier,
  set: (value) => updateValue('HP.HPmodifier', parseInt(value, 10) || 0)
});
const type = computed({ /* ... wie name ... */ get: () => props.modelValue.type, set: (v) => updateValue('type', v) });
const alignment = computed({ /* ... */ get: () => props.modelValue.alignment, set: (v) => updateValue('alignment', v) });
const languages = computed({ // Für v-combobox (erlaubt Freitexteingabe + Auswahl)
  get: () => {
      // Konvertiere String in Array für v-combobox, wenn nötig
      const langString = props.modelValue.languages || '';
      return langString.split(',').map(s => s.trim()).filter(s => s);
  },
  set: (value) => {
      // Konvertiere Array zurück in String
      updateValue('languages', Array.isArray(value) ? value.join(', ') : value || '');
  }
});
// Stats
const str = computed({ get: () => props.modelValue.stats?.STR, set: (v) => updateValue('stats.STR', Math.max(0, parseInt(v, 10) || 0)) });
const dex = computed({ get: () => props.modelValue.stats?.DEX, set: (v) => updateValue('stats.DEX', Math.max(0, parseInt(v, 10) || 0)) });
const con = computed({ get: () => props.modelValue.stats?.CON, set: (v) => updateValue('stats.CON', Math.max(0, parseInt(v, 10) || 0)) });
const int = computed({ get: () => props.modelValue.stats?.INT, set: (v) => updateValue('stats.INT', Math.max(0, parseInt(v, 10) || 0)) });
const wis = computed({ get: () => props.modelValue.stats?.WIS, set: (v) => updateValue('stats.WIS', Math.max(0, parseInt(v, 10) || 0)) });
const cha = computed({ get: () => props.modelValue.stats?.CHA, set: (v) => updateValue('stats.CHA', Math.max(0, parseInt(v, 10) || 0)) });
// AC
const ac = computed({ get: () => props.modelValue.AC, set: (v) => updateValue('AC', parseInt(v, 10) || 0) });
const acType = computed({ get: () => props.modelValue.ACType, set: (v) => updateValue('ACType', v) });
// Initiative
const initOverrideValue = computed({
    get: () => props.modelValue.Initiative?.initOverrideValue,
    set: (v) => {
        const newOverride = v === '' || v === null ? null : parseInt(v, 10);
        updateValue('Initiative.initOverrideValue', newOverride);
         // Aktualisiere den Toggle-State, falls der Wert manuell gelöscht wird
        isInitOverride.value = newOverride !== null;
    }
});
const initProficiency = computed({
    get: () => props.modelValue.Initiative?.initProficiency ?? false,
    set: (v) => updateValue('Initiative.initProficiency', v)
});
const initExpertise = computed({
    get: () => props.modelValue.Initiative?.initExpertise ?? false,
    set: (v) => updateValue('Initiative.initExpertise', v)
});


// Computed Properties für Anzeige
const strMod = computed(() => renderBonus(statModifier(str.value)));
const dexMod = computed(() => renderBonus(statModifier(dex.value)));
const conMod = computed(() => renderBonus(statModifier(con.value)));
const intMod = computed(() => renderBonus(statModifier(int.value)));
const wisMod = computed(() => renderBonus(statModifier(wis.value)));
const chaMod = computed(() => renderBonus(statModifier(cha.value)));

const initiativeDefaultValue = computed(() => props.modelValue.Initiative?.initDefaultValue ?? 0);
const isInitiativeOverridden = computed(() => props.modelValue.Initiative?.initOverrideValue !== null);
// const displayInitiativeBonus = computed(() => {
//    return isInitiativeOverridden.value ? initOverrideValue.value : initiativeDefaultValue.value;
// });


// --- Logik für den Triple-State Button (Initiative) ---
const initiativeProfState = computed(() => {
    if (initExpertise.value) return 'expertise'; // Höchste Prio
    if (initProficiency.value) return 'proficient';
    return 'none';
});

function cycleInitiativeProf() {
    let newProf = false; let newExp = false;
    if (initiativeProfState.value === 'none') { newProf = true; }
    else if (initiativeProfState.value === 'proficient') { newProf = true; newExp = true; }
    // Update beide Werte im Initiative-Objekt
    updateValue('Initiative', { ...props.modelValue.Initiative, initProficiency: newProf, initExpertise: newExp });
}

const initiativeButtonIcon = computed(() => {
     if (initiativeProfState.value === 'expertise') return 'mdi-chevron-double-up';
     if (initiativeProfState.value === 'proficient') return 'mdi-chevron-up';
     return 'mdi-minus';
});
const initiativeButtonTooltip = computed(() => {
     if (initiativeProfState.value === 'expertise') return 'Expertise';
     if (initiativeProfState.value === 'proficient') return 'Proficiency';
     return 'No Proficiency/Expertise';
});
// ------------------------------------------------------

// CR Daten für Select aufbereiten
const crDataListForSelect = computed(() => {
    return crDataList.value.map(cr => ({
        // Zeige CR-String und XP an
        title: `CR ${cr.cr} (${cr.xp.toLocaleString()} XP)`,
        // Der Wert des Selects soll der numerische CR sein
        value: cr.numeric
    }));
});

function toggleHDOverride() {
    isHDOverride.value = !isHDOverride.value;
    if (!isHDOverride.value) {
        // Wenn Override deaktiviert wird, setze overrideDie auf null
        updateValue('HP.overrideDie', null);
    } else {
         // Wenn Override aktiviert wird, setze den Override auf den aktuellen Default
         // oder einen sinnvollen Startwert, falls der Benutzer direkt ändern will.
         updateValue('HP.overrideDie', props.modelValue.HP?.defaultDie ?? 8);
    }
}

function toggleInitOverride() {
    isInitOverride.value = !isInitOverride.value;
    if (!isInitOverride.value) {
        // Wenn Override deaktiviert wird, setze overrideValue auf null
        updateValue('Initiative.initOverrideValue', null);
    }
    // Wenn aktiviert, kann der Benutzer direkt in das Feld tippen
}

</script>

<template>
  <div>
    <v-row dense>
        <!-- Zeile 1 -->
        <v-col cols="12" md="6">
            <v-text-field label="Name" v-model="name" density="compact" variant="outlined" clearable />
        </v-col>
        <v-col cols="12" md="2"> 
             <v-select label="CR" :items="crDataListForSelect" v-model="cr" density="compact" variant="outlined" clearable/>
        </v-col>
        <v-col cols="12" md="4">
             <!-- Leer für Referenz -->
        </v-col>

        <!-- Zeile 2 -->
        <v-col cols="12" md="2">
             <v-select label="Size" :items="sizes" v-model="size" density="compact" variant="outlined"/>
        </v-col>
        <v-col cols="12" md="2"> 
             <div class="d-flex align-center">
                 <v-text-field
                    v-if="!isHDOverride"
                    label="HD Size"
                    :model-value="`d${hpTypeDefault}`"
                    density="compact" variant="outlined" readonly
                    hint="Default based on size" persistent-hint
                 />
                 <v-select
                    v-else
                    label="HD Size Override"
                    :items="diceTypes"
                    v-model="hpTypeOverride"
                    density="compact" variant="outlined"
                    hide-details
                 />
                 <v-tooltip location="top" :text="isHDOverride ? 'Use Default HD Size' : 'Override HD Size'">
                     <template v-slot:activator="{ props: tooltipProps }">
                         <v-btn
                             v-bind="tooltipProps"
                             :icon="isHDOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                             variant="text" size="small" class="ml-1"
                             @click="toggleHDOverride"
                         />
                     </template>
                 </v-tooltip>
             </div>
        </v-col>
        <v-col cols="12" md="2">
            <v-number-input label="HD Count" v-model="hpHD" density="compact" variant="outlined" :min="1" control-variant="stacked" :reverse="false" inset/>
        </v-col>
         <v-col cols="12" md="2"> <!-- HP Modifier 4 breit -->
            <v-number-input label="HP Modifier" v-model="hpModifier" density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset/>
        </v-col>
        <v-col cols="12" md="4">
             <!-- Leer für Referenz -->
        </v-col>

        <!-- Zeile 3 -->
        <v-col cols="12" md="3">
             <v-select label="Type" :items="types" v-model="type" density="compact" variant="outlined" clearable/>
        </v-col>
        <v-col cols="12" md="3">
             <v-select label="Alignment" :items="alignments" v-model="alignment" density="compact" variant="outlined" clearable/>
        </v-col>
        <v-col cols="12" md="6">
             <!-- v-combobox erlaubt Freitexteingabe und Auswahl -->
             <v-combobox
                label="Languages"
                v-model="languages"
                :items="languagesList"
                multiple chips closable-chips deletable-chips
                density="compact"
                variant="outlined"
             />
             <v-tooltip
                activator="parent"
                location="top"
                >Enter custom languages or select from list</v-tooltip>
        </v-col>

         <!-- Zeile 4 (Stats) -->
         <v-col cols="12" md="2" v-for="statKey in ['STR', 'DEX', 'CON', 'INT', 'WIS', 'CHA']" :key="statKey">
            <v-number-input
                :label="statKey"
                :model-value="modelValue.stats?.[statKey]"
                @update:model-value="updateValue(`stats.${statKey}`, parseInt($event, 10) || 0)"
                density="compact"
                variant="outlined"
                :min="0"
                :max="30"
                control-variant="stacked"
                :reverse="false"
                inset
             >
                <template v-slot:append-inner>
                    <span class="text-caption text-disabled">({{ renderBonus(statModifier(modelValue.stats?.[statKey])) }})</span>
                </template>
             </v-number-input>
             <!-- ================================== -->
         </v-col>

         <!-- Zeile 5 (AC & Initiative) -->
         <v-col cols="12" md="2">
            <v-number-input label="AC" v-model="ac" density="compact" variant="outlined" :min="0" control-variant="stacked" :reverse="false" inset/>
         </v-col>
          <v-col cols="12" md="4">
             <!-- v-combobox erlaubt auch eigene Eingabe -->
             <v-combobox
                label="AC Type"
                :items="acTypes"
                v-model="acType"
                density="compact"
                variant="outlined"
                clearable
             />
         </v-col>
         <v-col cols="12" md="3"> <!-- Init Bonus Feld -->
             <div class="d-flex align-center">
                 <v-number-input
                     label="Init Bonus"
                     :model-value="isInitOverride ? initOverrideValue : initiativeDefaultValueDisplay"
                     @update:model-value="initOverrideValue = $event" 
                     density="compact" variant="outlined"
                     :readonly="!isInitOverride" 
                     :class="{ 'text-disabled': !isInitOverride }"
                     :hint="isInitOverride ? 'Manual override active' : 'Using default (DEX + Prof/Exp)'"
                     persistent-hint clearable
                     control-variant="stacked"
                     :reverse="false"
                     inset
                     class="left-aligned-input flex-grow-1" 
                 >
                      <!-- Triple-State Button im append-inner Slot -->
                     <template v-slot:append-inner>
                         <v-tooltip :text="initiativeButtonTooltip">
                           <template v-slot:activator="{ props: tooltipProps }">
                              <v-btn v-bind="tooltipProps" :icon="initiativeButtonIcon" variant="text" size="x-small" @click="cycleInitiativeProf" class="mr-1"/>
                           </template>
                         </v-tooltip>
                     </template>
                  </v-number-input>
                  <!-- Override Toggle Button -->
                  <v-tooltip location="top" :text="isInitOverride ? 'Use Default Initiative' : 'Override Initiative'">
                      <template v-slot:activator="{ props: tooltipProps }">
                          <v-btn
                              v-bind="tooltipProps"
                              :icon="isInitOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                              variant="text" size="small" class="ml-1"
                              @click="toggleInitOverride"
                          />
                      </template>
                  </v-tooltip>
             </div>
         </v-col>
         <v-col cols="12" md="3">
             <!-- Leer für Referenz -->
         </v-col>

    </v-row>
  </div>
</template>

<style scoped>
/* Optional: Hebe Overrides hervor */
.v-text-field:has(input:placeholder-shown:not(:focus)) + .v-input__details .v-messages {
    /* Style für Hint, wenn Placeholder (Default) angezeigt wird */
     color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity));
}
.v-text-field:not(:has(input:placeholder-shown)) + .v-input__details .v-messages {
    /* Style für Hint, wenn ein Override-Wert drin steht */
    color: rgb(var(--v-theme-info)); /* z.B. Blau */
    font-weight: bold;
}
/* === NEU: Stelle Input-Text linksbündig in v-number-input === */
/* Diese Regel muss möglicherweise angepasst werden, je nachdem, wie Vuetify */
/* die internen Klassen genau strukturiert. Untersuche das Element im Browser! */
:deep(.v-number-input .v-field__input) {
  text-align: left !important;
  /* Du musst evtl. auch das padding anpassen, wenn der Text zu nah an den Buttons ist */
  /* padding-inline-start: 8px !important; */
}
/* ============================================================ */
:deep(.left-aligned-input .v-field__input) {
  text-align: left !important;
  /* padding-right: 30px !important; */ /* Ggf. anpassen, da jetzt nur noch ein Button rechts ist */
}
:deep(.left-aligned-input .v-field__append-inner) {
    /* padding-inline-start: 0px !important; */ /* Entferne, falls die Buttons zu nah sind */
    align-items: center;
}
:deep(.left-aligned-input .v-field__append-inner .v-btn) {
    min-width: auto !important; /* Erlaube kleineren Button */
    padding: 0 4px !important; /* Kleineres Padding */
}
</style>