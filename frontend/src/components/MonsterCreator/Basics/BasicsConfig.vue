<!-- frontend/src/components/MonsterCreator/Basics/BasicsConfig.vue -->
<script setup>
import { computed, ref, onMounted, watch } from 'vue';
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

// Hilfsfunktion zum Updaten des modelValue Objekts
function updateValue(key, value) {
    // Spezialbehandlung für verschachtelte Objekte
    if (key.startsWith('HP.') || key.startsWith('stats.') || key.startsWith('Initiative.')) {
        const keys = key.split('.');
        const newObject = _.cloneDeep(props.modelValue); // Tiefe Kopie
        _.set(newObject, keys, value); // Setze den Wert im verschachtelten Objekt
        emit('update:modelValue', newObject);
    } else {
        // Normales Top-Level Update
        emit('update:modelValue', { ...props.modelValue, [key]: value });
    }
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
      // === NEU: Setze Default HD Size basierend auf Size ===
      const sizeData = sizes.value.find(s => s === value); // Finde Size-Daten
       if (sizeData && props.modelValue.HP.overrideDie === null) { // Nur wenn kein Override gesetzt ist
           const hdMapping = { 'Tiny': 4, 'Small': 6, 'Medium': 8, 'Large': 10, 'Huge': 12, 'Gargantuan': 20, 'Titan': 20 };
           const defaultDie = hdMapping[value] || 8; // Default zu 8 wenn unbekannt
           updateValue('HP.defaultDie', defaultDie);
           // Setze overrideDie zurück, falls jemand es manuell entfernt
           if (props.modelValue.HP.overrideDie === null && props.modelValue.HP.defaultDie !== defaultDie) {
               // Dieser Fall sollte selten sein, aber zur Sicherheit
               updateValue('HP.overrideDie', null);
           }
       }
      //======================================================
  }
});
const hpHD = computed({
  get: () => props.modelValue.HP?.HDAmount,
  set: (value) => updateValue('HP.HDAmount', parseInt(value, 10) || 1)
});
const hpTypeDefault = computed(() => props.modelValue.HP?.defaultDie); // Nur zum Anzeigen
const hpTypeOverride = computed({ // Für das Select-Feld
  get: () => props.modelValue.HP?.overrideDie, // Zeigt den Override-Wert an
  set: (value) => updateValue('HP.overrideDie', value ? parseInt(value, 10) : null) // Speichert Override oder null
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
const str = computed({ get: () => props.modelValue.stats?.STR, set: (v) => updateValue('stats.STR', parseInt(v, 10) || 0) });
const dex = computed({ get: () => props.modelValue.stats?.DEX, set: (v) => updateValue('stats.DEX', parseInt(v, 10) || 0) });
const con = computed({ get: () => props.modelValue.stats?.CON, set: (v) => updateValue('stats.CON', parseInt(v, 10) || 0) });
const int = computed({ get: () => props.modelValue.stats?.INT, set: (v) => updateValue('stats.INT', parseInt(v, 10) || 0) });
const wis = computed({ get: () => props.modelValue.stats?.WIS, set: (v) => updateValue('stats.WIS', parseInt(v, 10) || 0) });
const cha = computed({ get: () => props.modelValue.stats?.CHA, set: (v) => updateValue('stats.CHA', parseInt(v, 10) || 0) });
// AC
const ac = computed({ get: () => props.modelValue.AC, set: (v) => updateValue('AC', parseInt(v, 10) || 0) });
const acType = computed({ get: () => props.modelValue.ACType, set: (v) => updateValue('ACType', v) });
// Initiative
const initOverrideValue = computed({
    get: () => props.modelValue.Initiative?.initOverrideValue,
    set: (v) => updateValue('Initiative.initOverrideValue', v === '' || v === null ? null : parseInt(v, 10))
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
const displayInitiativeBonus = computed(() => {
    return isInitiativeOverridden.value ? initOverrideValue.value : initiativeDefaultValue.value;
});


// --- Logik für den Triple-State Button (Initiative) ---
const initiativeProfState = computed(() => {
    if (initExpertise.value) return 'expertise'; // Höchste Prio
    if (initProficiency.value) return 'proficient';
    return 'none';
});

function cycleInitiativeProf() {
    let newProf = false;
    let newExp = false;
    if (initiativeProfState.value === 'none') {
        newProf = true; // -> Proficient
    } else if (initiativeProfState.value === 'proficient') {
        newProf = true; // Bleibt Proficient
        newExp = true;  // -> Expertise
    } else { // War Expertise
        // -> None
    }
     // Wichtig: Beide gleichzeitig updaten, um korrekte Neuberechnung auszulösen
     const newInitiativeData = { ...props.modelValue.Initiative, initProficiency: newProf, initExpertise: newExp };
     updateValue('Initiative', newInitiativeData);
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


</script>

<template>
  <div>
    <v-row dense>
        <!-- Zeile 1 -->
        <v-col cols="12" md="6">
            <v-text-field label="Name" v-model="name" density="compact" variant="outlined" clearable />
        </v-col>
        <v-col cols="12" md="3"> <!-- CR nur 3 breit -->
             <v-select label="CR" :items="crDataListForSelect" v-model="cr" density="compact" variant="outlined" clearable/>
        </v-col>
        <v-col cols="12" md="3">
             <!-- Nur Anzeige des PB -->
             <v-text-field label="Prof. Bonus (PB)" :model-value="renderBonus(modelValue.PB)" density="compact" variant="outlined" readonly hint="Calculated from CR" persistent-hint/>
        </v-col>

        <!-- Zeile 2 -->
        <v-col cols="12" md="2">
             <v-select label="Size" :items="sizes" v-model="size" density="compact" variant="outlined"/>
        </v-col>
        <v-col cols="12" md="2">
             <!-- Zeige Default Die, wenn kein Override -->
             <v-text-field
                label="HD Size (Default)"
                :model-value="`d${hpTypeDefault}`"
                density="compact"
                variant="outlined"
                readonly
                :hint="hpTypeOverride === null ? '' : `Overridden by d${hpTypeOverride}`"
                persistent-hint
                :class="{ 'text-disabled': hpTypeOverride !== null }" />
        </v-col>
        <v-col cols="12" md="2">
             <v-select
                label="HD Override"
                :items="diceTypes"
                v-model="hpTypeOverride"
                density="compact"
                variant="outlined"
                clearable
                hint="Select to override default" persistent-hint/>
        </v-col>
        <v-col cols="12" md="2">
             <v-text-field type="number" label="HD Count" v-model.number="hpHD" density="compact" variant="outlined" min="1"/>
        </v-col>
         <v-col cols="12" md="4"> <!-- HP Modifier 4 breit -->
             <v-text-field type="number" label="HP Modifier" v-model.number="hpModifier" density="compact" variant="outlined"/>
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
                hint="Enter custom languages or select from list" persistent-hint
             />
        </v-col>

         <!-- Zeile 4 (Stats) -->
         <v-col cols="12" md="2" v-for="statKey in ['STR', 'DEX', 'CON', 'INT', 'WIS', 'CHA']" :key="statKey">
            <v-text-field
                type="number"
                :label="statKey"
                :model-value="modelValue.stats ? modelValue.stats[statKey] : 10"
                @update:model-value="updateValue(`stats.${statKey}`, parseInt($event, 10) || 0)"
                density="compact"
                variant="outlined"
                min="0"
            >
                <!-- Zeige Modifier daneben -->
                <template v-slot:append-inner>
                    <span class="text-caption text-disabled">({{ renderBonus(statModifier(modelValue.stats ? modelValue.stats[statKey] : 10)) }})</span>
                </template>
            </v-text-field>
         </v-col>

         <!-- Zeile 5 (AC & Initiative) -->
         <v-col cols="12" md="2">
            <v-text-field type="number" label="AC" v-model.number="ac" density="compact" variant="outlined" min="0"/>
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
             <v-text-field
                 type="number"
                 label="Init Bonus"
                 v-model.number="initOverrideValue"
                 density="compact" variant="outlined"
                 :placeholder="renderBonus(initiativeDefaultValue)"
                 :hint="isInitiativeOverridden ? 'Override active' : 'Using default (DEX)'"
                 persistent-hint
                 clearable
              >
                 <template v-slot:append-inner>
                    <v-tooltip :text="initiativeButtonTooltip">
                      <template v-slot:activator="{ props: tooltipProps }">
                         <v-btn
                            v-bind="tooltipProps"
                            :icon="initiativeButtonIcon"
                            variant="text" size="x-small"
                            @click="cycleInitiativeProf"
                            class="ml-1"
                         />
                      </template>
                    </v-tooltip>
                 </template>
              </v-text-field>
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
</style>