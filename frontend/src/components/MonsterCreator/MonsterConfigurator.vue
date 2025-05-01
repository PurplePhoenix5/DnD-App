<script setup>
import { ref } from 'vue';
import { get } from 'lodash';

import BasicsConfig from './Basics/BasicsConfig.vue';
import SavesConfig from './Saving Throws/SavingThrowsConfig.vue';
import SpeedsConfig from './Speeds/SpeedsConfig.vue';
import SkillsConfig from './Skills/SkillsConfig.vue';
import SensesConfig from './Senses/SensesConfig.vue';
import ResistancesAndImmunitiesConfig from './Resistances & Immunities/ResistancesAndImmunitiesConfig.vue';
import InventoryConfig from './Inventory/InventoryConfig.vue';
import TraitsConfig from './Traits/TraitsConfig.vue';
import SpellcastingConfig from './Spellcasting/SpellcastingConfig.vue';


const props = defineProps({
    monsterData: { type: Object, required: true },
    isEnabled: { type: Boolean, default: false },
     // allSkillsInfo von MonsterCreator empfangen (wie in vorheriger Antwort besprochen)
    allSkillsInfo: { type: Object, default: () => ({}) }
});

const emit = defineEmits(['update-monster-field']);

function handleFieldUpdate(payload) {
  console.log('MonsterConfigurator: Relaying update:', payload);
  emit('update-monster-field', { path: payload.key, value: payload.value });
}

// Daten für die Expansion Panels (Liste der Komponenten direkt hier)
const panels = ref([
  { id: 'basics', title: 'Basics', icon: 'mdi-clipboard-text-outline', path: 'basics', component: BasicsConfig },
  { id: 'saves', title: 'Saving Throws', icon: 'mdi-shield-half-full', path: 'saves', component: SavesConfig },
  { id: 'speeds', title: 'Speeds', icon: 'mdi-run-fast', path: 'speeds', component: SpeedsConfig  },
  { id: 'skills', title: 'Skills', icon: 'mdi-star-check-outline', path: 'skills', component: SkillsConfig },
  { id: 'senses', title: 'Senses', icon: 'mdi-eye-outline', path: 'senses', component: SensesConfig },
  { id: 'resistances', title: 'Resistances & Immunities', icon: 'mdi-shield-check-outline', path: 'resistImmun', component: ResistancesAndImmunitiesConfig },
  { id: 'inventory', title: 'Inventory', icon: 'mdi-treasure-chest-outline', path: 'inventory', component: InventoryConfig },
  { id: 'traits', title: 'Traits', icon: 'mdi-puzzle-outline', path: 'traits', component: TraitsConfig },
  { id: 'spellcasting', title: 'Spellcasting', icon: 'mdi-magic-staff', path: 'spellcasting', component: SpellcastingConfig,  },
  { id: 'actions', title: 'Actions', icon: 'mdi-sword' /* , component: ActionsConfig, path: 'actions' */ },
  { id: 'multiattack', title: 'Multi Attack', icon: 'mdi-plus-circle-multiple-outline' /* , component: MultiattackConfig, path: 'multiattacks' */ },
  { id: 'bonusactions', title: 'Bonus Actions', icon: 'mdi-sword-cross' /* , component: BonusActionsConfig, path: 'bonusAction' */ },
  { id: 'reactions', title: 'Reactions', icon: 'mdi-reply' /* , component: ReactionsConfig, path: 'reactions' */ },
  { id: 'legendary', title: 'Legendary Actions', icon: 'mdi-crown-outline' /* , component: LegendaryActionsConfig, path: 'legendaryActions' */ },
  { id: 'lair', title: 'Lair Actions', icon: 'mdi-castle' /* , component: LairActionsConfig, path: 'lairActions' */ },
]);

// Verwende lodash.get, um die Daten sicher zu extrahieren
const getDataForPanel = (panel) => {
    if (!panel.path) return undefined;

    if (panel.path === 'resistImmun') {
         // Diese Funktion gibt das Kombi-Objekt mit Defaults zurück
         return {
             resistances: get(props.monsterData, 'resistances', []),
             immunities: get(props.monsterData, 'immunities', []),
             vulnerabilities: get(props.monsterData, 'vulnerabilities', []),
             conditionImmunities: get(props.monsterData, 'conditionImmunities', [])
         };
    }
    if (panel.path === 'actions' || panel.path === 'bonusAction') {
         // Gehe davon aus, dass es bereits ein Array im monsterData ist
         return get(props.monsterData, panel.path, []); // Default zu leerem Array
    }

    return get(props.monsterData, panel.path, undefined);
};

// Funktion zur sicheren Übergabe von Defaults
const getModelValueForComponent = (panel) => {
    const data = getDataForPanel(panel);
    // Defaults für verschiedene Typen
    if (panel.path === 'basics') return data ?? {};
    if (panel.path === 'saves') return data ?? {};
    if (panel.path === 'speeds') return data ?? [];
    if (panel.path === 'skills') return data ?? [];
    if (panel.path === 'senses') return data ?? {};
    if (panel.path === 'resistImmun') return data;
    if (panel.path === 'inventory') return data ?? '';
    if (panel.path === 'traits') return data ?? [];
    if (panel.path === 'spellcasting') return data ?? [];
    if (panel.path === 'actions' || panel.path === 'bonusAction') return data ?? [];

    // Fallback: Standardmäßig das geholte Datum zurückgeben, oder undefined
    return data;
};


</script>


<template>
  <v-expansion-panels variant="popout" multiple>
    <v-expansion-panel
      v-for="panel in panels"
      :key="panel.id"
      :value="panel.id"
      elevation="1"
      :disabled="panel.id !== 'basics' && !isEnabled && !panel.component && panel.id !== 'speeds' && panel.id !== 'skills' && panel.id !== 'senses' && panel.id !== 'resistances' && panel.id !== 'inventory' && panel.id !== 'traits'"
      >
      <v-expansion-panel-title>
        <v-icon :icon="panel.icon" start class="mr-2"></v-icon>
        {{ panel.title }}
      </v-expansion-panel-title>

      <v-expansion-panel-text eager>
        <!-- Verwende v-if für jeden spezifischen Fall -->
        <BasicsConfig v-if="panel.id === 'basics'"
                      :modelValue="getModelValueForComponent(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <SavesConfig v-else-if="panel.id === 'saves'"
                     :modelValue="getModelValueForComponent(panel)"
                     :is-enabled="isEnabled"
                     @update:field="handleFieldUpdate($event)" />
        <SpeedsConfig v-else-if="panel.id === 'speeds'"
                      :modelValue="getModelValueForComponent(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <!-- SkillsConfig braucht basicsData -> füge ?? {} hinzu -->
        <SkillsConfig v-else-if="panel.id === 'skills'"
                      :modelValue="getModelValueForComponent(panel)"
                      :basicsData="getDataForPanel({ path: 'basics' }) ?? {}" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <!-- SensesConfig braucht basicsData, skillsData, allSkillsInfo -->
        <SensesConfig v-else-if="panel.id === 'senses'"
                      :modelValue="getModelValueForComponent(panel)"
                      :basicsData="getDataForPanel({ path: 'basics' }) ?? {}" 
                      :skillsData="getModelValueForComponent({ path: 'skills' }) ?? []" 
                      :allSkillsInfo="allSkillsInfo" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <ResistancesAndImmunitiesConfig v-else-if="panel.id === 'resistances'"
                      :modelValue="getModelValueForComponent(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <InventoryConfig v-else-if="panel.id === 'inventory'"
                      :modelValue="getModelValueForComponent(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <TraitsConfig v-else-if="panel.id === 'traits'"
                      :modelValue="getModelValueForComponent(panel)"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <SpellcastingConfig v-else-if="panel.id === 'spellcasting'"
                      :modelValue="getModelValueForComponent(panel)"
                      :basicsData="getDataForPanel({ path: 'basics' }) ?? {}"
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)" />
        <ActionsConfig v-else-if="panel.id === 'actions'"
                      :modelValue="getModelValueForComponent(panel)" 
                      :basicsData="getDataForPanel({ path: 'basics' }) ?? {}" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)"
                 />
        <BonusActionsConfig v-else-if="panel.id === 'bonusactions'"
                      :modelValue="getModelValueForComponent(panel)" 
                      :basicsData="getDataForPanel({ path: 'basics' }) ?? {}" 
                      :is-enabled="isEnabled"
                      @update:field="handleFieldUpdate($event)"
                 />
        <p v-else class="text-disabled pa-4">
          Configuration for {{ panel.title }} not implemented yet.
        </p>
      </v-expansion-panel-text>
    </v-expansion-panel>
  </v-expansion-panels>
  <div v-if="!isEnabled" class="text-disabled text-center mt-4 text-caption">
      Please enter Monster Name and CR in the "Basics" section to enable configuration.
  </div>
</template>

<style scoped>
/* Styles bleiben unverändert */
.splitpanes.default-theme {
  /* height: calc(100vh - 200px);  */ /* Höhe im style-Attribut von Splitpanes gesetzt */
}
.splitpanes.default-theme .splitpanes__splitter {
  background-color: #000000;
  position: relative;
}
.splitpanes.default-theme .splitpanes__splitter:before {
  content: '';
  position: absolute;
  left: 0;
  top: 0;
  transition: opacity 0.4s;
  background-color: rgba(0, 0, 0, 0.2);
  opacity: 0;
  z-index: 1;
}
.splitpanes.default-theme .splitpanes__splitter:hover:before {
  opacity: 1;
}
.splitpanes--vertical > .splitpanes__splitter:before {
  left: -1px;
  right: -1px;
  height: 100%;
}
.splitpanes--horizontal > .splitpanes__splitter:before {
  top: -1px;
  bottom: -1px;
  width: 100%;
}
</style>