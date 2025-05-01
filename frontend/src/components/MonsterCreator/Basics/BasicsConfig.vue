<!-- frontend/src/components/MonsterCreator/Basics/BasicsConfig.vue -->
<script setup>
import { computed, ref, onMounted, watch } from 'vue';
import { loadDnDData } from '../../../utils/dndDataService.js'; 
import { statModifier, calculateInitiativeBonus, renderBonus } from '../../../utils/mathRendering.js'; 

const props = defineProps({
  modelValue: { type: Object, required: true },
});

const emit = defineEmits(['update:field']);



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
watch(() => props.modelValue.HP?.overrideDie, (newVal) => { isHDOverride.value = newVal !== null; });
watch(() => props.modelValue.Initiative?.initOverrideValue, (newVal) => { isInitOverride.value = newVal !== null; });

function updateField(key, value) {
    // Beispiel: key = 'name', 'CR', 'HP.HDAmount', 'stats.STR', 'Initiative.initProficiency'
    console.log(`BasicsConfig: Emitting update:field - Key: ${key}, Value:`, value);
    emit('update:field', { key: key, value: value });
}

// --- Computed Properties mit Getter/Setter für v-model ---
const name = computed({
  get: () => props.modelValue.name,
  set: (value) => updateField('name', value)
});
const cr = computed({
  get: () => props.modelValue.CR, // Passe den Pfad hier an
  set: (value) => {
    updateField('CR', value); 
  }
});
const size = computed({
  get: () => props.modelValue.size,
  set: (value) => {
      updateField('size', value);
  }
});
const hpHD = computed({
  get: () => props.modelValue.HP?.HDAmount,
  set: (value) => updateField('HP.HDAmount', Math.max(1, parseInt(value, 10) || 1))
});
const displayedHpType = computed(() => {
    return isHDOverride.value ? props.modelValue.HP?.overrideDie : props.modelValue.HP?.defaultDie;
});
const hpModifier = computed({
  get: () => props.modelValue.HP?.HPmodifier,
  set: (value) => updateField('HP.HPmodifier', parseInt(value, 10) || 0)
});
const type = computed({ get: () => props.modelValue.type, set: (v) => updateField('type', v) });
const alignment = computed({ get: () => props.modelValue.alignment, set: (v) => updateField('alignment', v) });
const languages = computed({
  get: () => (props.modelValue.languages || '').split(',').map(s=>s.trim()).filter(s=>s),
  set: (value) => updateField('languages', Array.isArray(value) ? value.join(', ') : '')
});
// Stats
const str = computed({ get: () => props.modelValue.stats?.STR, set: (v) => updateField('stats.STR', Math.max(0, parseInt(v, 10) || 0)) });
const dex = computed({ get: () => props.modelValue.stats?.DEX, set: (v) => updateField('stats.DEX', Math.max(0, parseInt(v, 10) || 0)) });
const con = computed({ get: () => props.modelValue.stats?.CON, set: (v) => updateField('stats.CON', Math.max(0, parseInt(v, 10) || 0)) });
const int = computed({ get: () => props.modelValue.stats?.INT, set: (v) => updateField('stats.INT', Math.max(0, parseInt(v, 10) || 0)) });
const wis = computed({ get: () => props.modelValue.stats?.WIS, set: (v) => updateField('stats.WIS', Math.max(0, parseInt(v, 10) || 0)) });
const cha = computed({ get: () => props.modelValue.stats?.CHA, set: (v) => updateField('stats.CHA', Math.max(0, parseInt(v, 10) || 0)) });
// AC
const ac = computed({ get: () => props.modelValue.AC, set: (v) => updateField('AC', parseInt(v, 10) || 0) });
const acType = computed({ get: () => props.modelValue.ACType, set: (v) => updateField('ACType', v || '') });
// Initiative
const initOverrideValue = computed({
    get: () => props.modelValue.Initiative?.initOverrideValue,
    set: (v) => {
        const newOverride = v === '' || v === null ? null : parseInt(v, 10);
        updateField('Initiative.initOverrideValue', newOverride);
        isInitOverride.value = newOverride !== null;
    }
});
const displayedInitiativeBonus = computed(() => {
    return isInitOverride.value
        ? props.modelValue.Initiative?.initOverrideValue 
        : props.modelValue.Initiative?.initDefaultValue;  
});
const initProficiency = computed({
    get: () => props.modelValue.Initiative?.initProficiency ?? false,
    set: (v) => updateField('Initiative.initProficiency', v)
});
const initExpertise = computed({
    get: () => props.modelValue.Initiative?.initExpertise ?? false,
    set: (v) => updateField('Initiative.initExpertise', v)
});


// Computed Properties für Anzeige
const strMod = computed(() => renderBonus(statModifier(str.value)));
const dexMod = computed(() => renderBonus(statModifier(dex.value)));
const conMod = computed(() => renderBonus(statModifier(con.value)));
const intMod = computed(() => renderBonus(statModifier(int.value)));
const wisMod = computed(() => renderBonus(statModifier(wis.value)));
const chaMod = computed(() => renderBonus(statModifier(cha.value)));
const isInitiativeOverridden = computed(() => props.modelValue.Initiative?.initOverrideValue !== null);

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
    // === Sende separate Updates für die Flags ===
    updateField('Initiative.initProficiency', newProf);
    updateField('Initiative.initExpertise', newExp);
    // ===========================================
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
        title: `${cr.cr} (${cr.xp.toLocaleString()} XP)`,
        // Der Wert des Selects soll der numerische CR sein
        value: cr.numeric
    }));
});

function toggleHDOverride() {
    const nextState = !isHDOverride.value;
    isHDOverride.value = nextState; 
    if (!nextState) { updateField('HP.overrideDie', null); }
    else { updateField('HP.overrideDie', props.modelValue.HP?.defaultDie ?? 8); }
}

function toggleInitOverride() {
    const nextState = !isInitOverride.value;
    isInitOverride.value = nextState; 
    if (!nextState) { updateField('Initiative.initOverrideValue', null); }
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
             <v-select label="CR" :items="crDataListForSelect" v-model="cr" density="compact" variant="outlined"/>
        </v-col>
        <v-col cols="12" md="4">
             <!-- Leer für Referenz -->
        </v-col>

        <!-- Zeile 2 -->
        <v-col cols="12" md="2">
             <v-select label="Size" :items="sizes" v-model="size" density="compact" variant="outlined"/>
        </v-col>
        <v-col cols="12" md="2">
             <v-select
                :label="isHDOverride ? 'HD Size' : 'HD Size'"
                :items="diceTypes"
                :model-value="displayedHpType" 
                @update:model-value="updateField('HP.overrideDie', $event === null ? null : Number($event))"
                density="compact" variant="outlined"
                :readonly="!isHDOverride" 
                :class="{'input-is-default': !isHDOverride}" 
             >
                 <!-- Override Toggle Button im Slot -->
                 <template v-slot:append-inner>
                     <v-tooltip location="top" :text="isHDOverride ? 'Use Default' : 'Use Override'">
                         <template v-slot:activator="{ props: tooltipProps }">
                             <v-btn
                                 v-bind="tooltipProps"
                                 :icon="isHDOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                                 variant="text" size="small"
                                 @click.stop="toggleHDOverride" 
                             />
                         </template>
                     </v-tooltip>
                 </template>
             </v-select>
        </v-col>
        <v-col cols="12" md="2">
            <v-number-input label="HD Count" v-model="hpHD" :model-value="modelValue.HP?.HDAmount" @update:model-value="updateField('HP.HDAmount', Math.max(1, parseInt($event, 10) || 1))" density="compact" variant="outlined" :min="1" control-variant="stacked" :reverse="false" inset/>
        </v-col>
         <v-col cols="12" md="2"> <!-- HP Modifier 4 breit -->
            <v-number-input label="HP Modifier" v-model="hpModifier" :model-value="modelValue.HP?.HPmodifier" @update:model-value="updateField('HP.HPmodifier', parseInt($event, 10) || 0)" density="compact" variant="outlined" control-variant="stacked" :reverse="false" inset/>
        </v-col>
        <v-col cols="12" md="4">
             <!-- Leer für Referenz -->
        </v-col>

        <!-- Zeile 3 -->
        <v-col cols="12" md="3">
             <v-select label="Type" :items="types" v-model="type" density="compact" variant="outlined"/>
        </v-col>
        <v-col cols="12" md="3">
             <v-select label="Alignment" :items="alignments" v-model="alignment" density="compact" variant="outlined"/>
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
                @update:model-value="updateField(`stats.${statKey}`, parseInt($event, 10) || 0)"
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
             <v-number-input
                 label="Init Bonus"
                 :model-value="displayedInitiativeBonus" 
                 @update:model-value="updateField('Initiative.initOverrideValue', $event === '' || $event === null ? null : parseInt($event, 10))"
                 density="compact" variant="outlined"
                 :readonly="!isInitOverride" 
                 :class="{'input-is-default': !isInitOverride}" 
                 control-variant="stacked"
                 :reverse="false"
                 inset
                 class="left-aligned-input"
              >
                 <!-- Buttons im append-inner Slot -->
                 <template v-slot:append-inner>
                     <div class="d-flex align-center">
                         <!-- Triple-State Button -->
                         <v-tooltip :text="initiativeButtonTooltip">
                           <template v-slot:activator="{ props: tooltipProps }">
                              <v-btn v-bind="tooltipProps" :icon="initiativeButtonIcon" variant="text" size="x-small" @click="cycleInitiativeProf" class="mr-1"/>
                           </template>
                         </v-tooltip>
                         <!-- Override Toggle Button -->
                         <v-tooltip location="top" :text="isInitOverride ? 'Use Default' : 'Use Override'">
                             <template v-slot:activator="{ props: tooltipProps }">
                                 <v-btn
                                     v-bind="tooltipProps"
                                     :icon="isInitOverride ? 'mdi-lock-open-variant-outline' : 'mdi-lock-outline'"
                                     variant="text" size="x-small"
                                     @click.stop="toggleInitOverride" 
                                 />
                             </template>
                         </v-tooltip>
                     </div>
                 </template>
              </v-number-input>
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
    margin: 0 1px; /* Kleiner Abstand zwischen Buttons */
}
.input-is-default :deep(.v-field__input) {
  color: rgba(var(--v-theme-on-surface), var(--v-medium-emphasis-opacity)) !important;
  /*font-style: italic;*/
}
</style>