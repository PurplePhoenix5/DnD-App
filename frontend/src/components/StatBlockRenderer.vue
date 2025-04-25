<!-- frontend/src/components/StatBlockRenderer.vue -->
<template>
    <!-- Zeige Ladezustand für D&D Basisdaten ODER Spells -->
    <div v-if="isLoadingStaticData || isLoadingSpells" class="text-center pa-4">
      <v-progress-circular indeterminate color="primary"></v-progress-circular>
      <p class="mt-2 text-caption">Loading {{ isLoadingStaticData ? 'D&D data' : 'spell data' }}...</p>
    </div>
    <!-- Zeige den Statblock, wenn alle Daten geladen sind und Monsterdaten existieren -->
    <div v-else-if="monster && staticDataLoaded" id="render" class="statblock" :class="blockStyle" :style="columnStyle">
      <!-- Name -->
      <h2 class="monster-name">{{ monster.basics?.name || 'Unnamed Creature' }}</h2>
      <!-- Type, Size, Alignment (Zugriff über basics) -->
      <div class="type">
        {{ monster.basics?.size || 'Medium' }} {{ monster.basics?.type || 'unknown type' }}, {{ monster.basics?.alignment || 'unaligned' }}
      </div>
      <hr />
      <!-- AC / Initiative -->
      <div v-if="is2024" class="skill ac">
         <div>
           <span class="name">Armor Class</span> {{ monster.basics?.AC ?? 10 }}{{ monster.basics?.ACType ? ` (${monster.basics.ACType})` : '' }}
         </div>
         <span v-if="initiative"><b>Initiative</b> {{ initiative }}</span>
      </div>
       <div v-if="is2014" class="skill">
         <span class="name">Armor Class</span> {{ monster.basics?.AC ?? 10 }}{{ monster.basics?.ACType ? ` (${monster.basics.ACType})` : '' }}
       </div>
      <!-- HP -->
      <div class="skill"><span class="name">Hit Points</span> {{ hp }}</div>
      <!-- Speed -->
      <div class="skill"><span class="name">Speed</span> {{ speeds }}</div>
      <hr />
  
      <!-- Stats -->
      <div v-if="is2014" class="row no-wrap stats-2014" style="width: 100%">
         <div v-for="stat in stats2014" :key="stat.stat" class="stat-container">
           <div class="stat-name">{{ stat.stat }}</div>
           <div class="stat">
             <div class="score">{{ stat.score }}</div>
             <div class="modifier">({{ stat.renderedModifier }})</div>
           </div>
         </div>
       </div>
      <div v-if="is2024" class="row no-wrap stats" style="width: 100%">
         <div class="stat-table">
           <div class="header-label mod">Mod</div> <div class="header-label save">Save</div>
           <div class="stat one">STR</div> <div class="score one">{{ statsAndSavesByKey.STR?.score }}</div> <div class="mod one">{{ statsAndSavesByKey.STR?.renderedModifier }}</div> <div class="save one">{{ statsAndSavesByKey.STR?.renderedSave }}</div>
           <div class="stat two">INT</div> <div class="score two">{{ statsAndSavesByKey.INT?.score }}</div> <div class="mod two">{{ statsAndSavesByKey.INT?.renderedModifier }}</div> <div class="save two">{{ statsAndSavesByKey.INT?.renderedSave }}</div>
         </div>
         <div class="stat-table">
            <div class="header-label mod">Mod</div> <div class="header-label save">Save</div>
            <div class="stat one">DEX</div> <div class="score one">{{ statsAndSavesByKey.DEX?.score }}</div> <div class="mod one">{{ statsAndSavesByKey.DEX?.renderedModifier }}</div> <div class="save one">{{ statsAndSavesByKey.DEX?.renderedSave }}</div>
            <div class="stat two">WIS</div> <div class="score two">{{ statsAndSavesByKey.WIS?.score }}</div> <div class="mod two">{{ statsAndSavesByKey.WIS?.renderedModifier }}</div> <div class="save two">{{ statsAndSavesByKey.WIS?.renderedSave }}</div>
          </div>
         <div class="stat-table">
           <div class="header-label mod">Mod</div> <div class="header-label save">Save</div>
           <div class="stat one">CON</div> <div class="score one">{{ statsAndSavesByKey.CON?.score }}</div> <div class="mod one">{{ statsAndSavesByKey.CON?.renderedModifier }}</div> <div class="save one">{{ statsAndSavesByKey.CON?.renderedSave }}</div>
           <div class="stat two">CHA</div> <div class="score two">{{ statsAndSavesByKey.CHA?.score }}</div> <div class="mod two">{{ statsAndSavesByKey.CHA?.renderedModifier }}</div> <div class="save two">{{ statsAndSavesByKey.CHA?.renderedSave }}</div>
         </div>
      </div>
      <hr />
  
      <!-- Saves (Explicit 2014) -->
      <div v-if="is2014 && saves2014" class="skill">
        <span class="name">Saving Throws</span> {{ saves2014 }}
      </div>
      <!-- Skills -->
      <div v-show="skills" class="skill">
        <span class="name">Skills</span> {{ skills }}
      </div>
      <!-- Gear/Inventory -->
      <div v-if="is2024 && inventory" class="skill"> <span class="name">Gear</span> <span v-html="inventory"></span> </div>
      <!-- Resistances -->
      <div v-show="resistances" class="skill"> <span class="name">{{ resistancesLabel }}</span> {{ resistances }} </div>
      <!-- Immunities -->
       <div v-if="is2024 && immunitiesAndConditions" class="skill"> <span class="name">Damage & Condition Immunities</span> {{ immunitiesAndConditions }} </div>
       <div v-if="is2014 && immunities" class="skill"> <span class="name">{{ immunitiesLabel }}</span> {{ immunities }} </div>
      <!-- Vulnerabilities -->
      <div v-show="vulnerabilities" class="skill"> <span class="name">{{ vulnerabilitiesLabel }}</span> {{ vulnerabilities }} </div>
      <!-- Conditions (Explicit 2014) -->
      <div v-if="is2014 && conditions" class="skill"> <span class="name">Condition Immunities</span> {{ conditions }} </div>
      <!-- Senses -->
      <div v-show="senses" class="skill"> <span class="name">Senses</span> {{ senses }} </div>
      <!-- Languages -->
      <div class="skill"> <span class="name">Languages</span> {{ monster.basics?.languages || '—' }} </div>
      <!-- Challenge -->
      <div class="skill">
        <span class="name">Challenge</span> {{ cr }}
        <span v-if="is2014" style="float: right"> <b>Proficiency Bonus</b> {{ renderBonus(proficiencyBonus) }} </span>
      </div>
      <hr />
  
      <!-- Traits -->
      <template v-if="traits && traits.length > 0">
        <h3 class="section" :class="{ first: is2024 }">Traits</h3>
        <div class="traits"> <div v-for="(trait, idx) in traits" :key="`trait-${idx}`" class="trait" v-html="trait"></div> </div>
      </template>
  
      <!-- Actions -->
      <template v-if="actions && actions.length > 0">
          <h3 class="section">Actions</h3>
          <div v-if="multiattacks" class="multiattack" v-html="multiattacks"></div>
          <!-- Attack Roll Actions -->
          <div v-for="(attack, idx) in attackRollActions" :key="`attack-${idx}`" class="attack" v-html="attack"></div>
          <!-- Saving Throw Actions -->
          <div v-for="(action, idx) in savingThrowActions" :key="`action-save-${idx}`" class="action" v-html="action"></div>
          <!-- Other Actions -->
          <div v-for="(action, idx) in otherActions" :key="`action-other-${idx}`" class="action" v-html="action"></div>
      </template>
  
      <!-- Innate Spellcasting -->
       <div v-if="resolvedInnateSpellcastingLists && resolvedInnateSpellcastingLists.length > 0" class="innate-spellcasting"> /* ... unverändert ... */ </div>
  
      <!-- Class Spellcasting (Nicht im neuen Template? Vorerst auskommentiert) -->
      <!-- <div v-if="(resolvedClassSpellcastingSlots && resolvedClassSpellcastingSlots.length > 0) || (resolvedKnownSpellsByLevel[0] && resolvedKnownSpellsByLevel[0].length > 0)" class="spellcasting"> ... </div> -->
  
      <!-- Bonus Actions -->
      <template v-if="(bonusAttackRollActions && bonusAttackRollActions.length > 0) || (bonusSavingThrowActions && bonusSavingThrowActions.length > 0) || (bonusOtherActions && bonusOtherActions.length > 0)">
          <h3 class="section">Bonus Actions</h3>
          <div v-for="(attack, idx) in bonusAttackRollActions" :key="`bonus-attack-${idx}`" class="attack" v-html="attack"></div>
          <div v-for="(action, idx) in bonusSavingThrowActions" :key="`bonus-action-save-${idx}`" class="action" v-html="action"></div>
          <div v-for="(action, idx) in bonusOtherActions" :key="`bonus-action-other-${idx}`" class="action" v-html="action"></div>
      </template>
  
      <!-- Legendary Actions -->
      <template v-if="legendaryActions && legendaryActions.length > 0"> /* ... unverändert ... */ </template>
  
      <!-- Mythic Actions (Nicht im neuen Template? Vorerst auskommentiert) -->
      <!-- <template v-if="mythicActions && mythicActions.length > 0"> ... </template> -->
  
      <!-- Reactions -->
      <template v-if="reactions && reactions.length > 0">
         <h3 class="section">Reactions</h3>
         <div class="reactions"> <div v-for="(reaction, idx) in reactions" :key="`reaction-${idx}`" class="action reaction" v-html="reaction"></div> </div>
      </template>
  
      <!-- Lair Actions -->
      <template v-if="lairActions && lairActions.length > 0"> /* ... unverändert ... */ </template>
  
      <!-- Regional Effects (Nicht im neuen Template?) -->
      <!-- <template v-if="regionalEffects && regionalEffects.length > 0"> ... </template> -->
  
      <!-- Inventory (2014 Style at the bottom) -->
      <div v-if="is2014 && inventory" class="inventory"> <h3 class="section">Inventory</h3> <div v-html="inventory"></div> </div>
  
    </div>
     <!-- Fallback, wenn Monsterdaten fehlen oder statische Daten noch laden -->
    <div v-else-if="!staticDataLoaded"> Loading essential D&D data... </div>
    <div v-else> Monster data is missing or incomplete. </div>
  </template>
  
  <script setup>
  import { computed, defineProps, watch, ref, reactive, onMounted } from 'vue'; // onMounted hinzugefügt
  import _ from 'lodash';
  import N2W from 'number-to-words';
  
  // === Importiere den Daten-Lade-Service ===
  import { loadDnDData } from '../utils/dndDataService.js'; // Pfad prüfen
  // === Importiere NUR die async Spelldaten-Funktion ===
  import { getSpellData } from '../utils/spellsData.js'; // Pfad prüfen
  // Importiere die Math-Funktionen
  import {
      renderBonus, statModifier, avgRoll, avgHP,
      saveModifierForStat, // Behalte diese, wenn sie monster object & stat key braucht
      // bonusForSkill, // Wird unten neu definiert
      // saveForAction, // Wird unten neu definiert
      bonusForAttack, // Wird unten neu definiert
      bonusForAttackDamage, // Wird unten neu definiert
      // bonusForConditionalDamage // Nicht im neuen Template?
  } from '../utils/mathRendering.js';
  
  // --- Props ---
  const props = defineProps({
      monsterData: { type: Object, required: true, default: () => ({}) },
      columns: { type: Number, default: 1 },
      displayStyle: { type: String, default: '2024', validator: (v) => ['2014', '2024'].includes(v) }
  });
  
  // === Refs für geladene statische DnD-Daten ===
  const isLoadingStaticData = ref(true);
  const staticDataLoaded = ref(false); // Flag, um Rendering zu steuern
  const crData = ref([]);
  const skillsData = ref({}); // Wird { SKILL_KEY: { stat: '...', label: '...' } } sein
  const statsData = ref({}); // Wird { STAT_KEY: { full: '...' } } sein
  const attackRangesData = ref({}); // Wird { key: 'label' } sein (z.B. { MELEE: 'Melee' })
  const rechargeData = ref([]); // Wird Array sein
  // ... füge Refs für andere benötigte Daten hinzu (classes, etc.) ...
  // const classesData = ref({});
  // ================================================
  
  // === Refs für Spelldaten (wie zuvor) ===
  const isLoadingSpells = ref(false);
  const resolvedSpellDetails = ref({});
  // ========================================
  
  // Computed Property für das Monster-Objekt (jetzt mit sicherem Zugriff auf basics)
  const monster = computed(() => props.monsterData);
  const basics = computed(() => props.monsterData.basics || {}); // Sicherer Zugriff auf basics
  
  // Computed Properties für Style und Spalten (wie zuvor)
  const is2014 = computed(() => props.displayStyle === '2014');
  const is2024 = computed(() => props.displayStyle === '2024');
  const blockStyle = computed(() => ({ 'mm2014': is2014.value, 'mm2024': is2024.value }));
  const columnStyle = computed(() => ({ columnCount: [1, 2].includes(props.columns) ? props.columns : 1 }));
  
  // === Lade statische Daten beim Mounten ===
  onMounted(async () => {
      isLoadingStaticData.value = true;
      try {
          const [cr, skills, stats, attackRanges, recharge] = await Promise.all([
              loadDnDData('crData.json'),
              loadDnDData('skills.json'),
              loadDnDData('stats.json'),
              loadDnDData('attackRanges.json'), // Annahme: Datei existiert
              loadDnDData('recharge.json'),     // Annahme: Datei existiert
              // Lade weitere statische Daten hier
          ]);
          crData.value = cr || [];
          skillsData.value = skills || {};
          statsData.value = stats || {};
          attackRangesData.value = (attackRanges || []).reduce((acc, val) => { acc[val.toUpperCase()] = val; return acc; }, {}); // Konvertiere Array zu Objekt für einfachen Zugriff
          rechargeData.value = recharge || []; // Array lassen
  
          console.log("Static D&D data loaded.");
          staticDataLoaded.value = true; // Setze Flag nach erfolgreichem Laden
      } catch (error) {
          console.error("Failed to load static D&D data:", error);
          // Optional: Fehlermeldung anzeigen
      } finally {
          isLoadingStaticData.value = false;
      }
  });
  // =====================================
  
  // === Watcher und Funktion zum Laden der Spells (wie zuvor) ===
  watch(() => props.monsterData?.basics?.spellcasting, (newSpellcasting) => { // Beobachte die neue Struktur
      fetchRelevantSpellData(newSpellcasting);
  }, { immediate: true, deep: true });
  
  async function fetchRelevantSpellData(spellcastingData) {
      if (!spellcastingData || (!spellcastingData.atWill?.length && !spellcastingData.once?.length && !spellcastingData.twice?.length && !spellcastingData.thrice?.length)) {
          resolvedSpellDetails.value = {}; isLoadingSpells.value = false; return;
      }
      isLoadingSpells.value = true;
      const neededSpellNames = new Set();
      (spellcastingData.atWill || []).forEach(s => neededSpellNames.add(s.name));
      (spellcastingData.once || []).forEach(s => neededSpellNames.add(s.name));
      (spellcastingData.twice || []).forEach(s => neededSpellNames.add(s.name));
      (spellcastingData.thrice || []).forEach(s => neededSpellNames.add(s.name));
  
      if (neededSpellNames.size === 0) { resolvedSpellDetails.value = {}; isLoadingSpells.value = false; return; }
      console.log(`StatBlockRenderer: Fetching details for ${neededSpellNames.size} spells...`);
      const spellPromises = Array.from(neededSpellNames).map(async (name) => { /* ... wie zuvor ... */ });
      try { /* ... Promise.all und Mapping ... */ }
      catch (error) { /* ... Fehlerbehandlung ... */ }
      finally { isLoadingSpells.value = false; }
  }
  // =======================================================
  
  // === Hilfsfunktionen, die jetzt geladene Daten verwenden ===
  function getCrByNumeric(numericCR) {
      return crData.value.find(cr => cr.numeric === numericCR) || null;
  }
  function getSkillLabel(skillKey) {
      return skillsData.value[skillKey]?.label || skillKey; // Verwende Key als Fallback
  }
  function getSkillStat(skillKey) {
      return skillsData.value[skillKey]?.stat || 'INT'; // Default INT?
  }
  function getStatFullName(statKey) {
      return statsData.value[statKey]?.full || statKey;
  }
  function getAttackRangeName(rangeKey) {
       return attackRangesData.value[rangeKey?.toUpperCase()] || rangeKey || 'melee';
  }
  // getRechargeLabel muss jetzt auf rechargeData zugreifen (oder bleibt als Utility, wenn Logik einfach)
  function getRechargeLabelLocal(rate) {
      // Annahme: recharge.json ist ein Array von Strings wie ["(5-6)", "(Recharges after a Short or Long Rest)"]
      // Finde den passenden Eintrag oder gib den rohen Wert zurück.
      // Diese Logik hängt stark vom Format deiner recharge.json ab!
      // Beispiel:
      // const found = rechargeData.value.find(r => r.key === rate); return found ? found.label : rate;
      return rate || 'day'; // Einfacher Fallback
  }
  // =========================================================
  
  // === Computed Properties (angepasst an neue Struktur und geladene Daten) ===
  
  const proficiencyBonus = computed(() => basics.value?.PB ?? 2);
  
  const calculatedAvgHp = computed(() => {
      if (!basics.value?.HP) return 1;
      const HP = basics.value.HP;
      // Verwende overrideDie, wenn vorhanden, sonst defaultDie
      const dieType = HP.overrideDie ?? HP.defaultDie ?? 8;
      return Math.max(1, Math.floor(HP.HDAmount * ((dieType + 1) / 2) + HP.HPmodifier));
  });
  
  const hp = computed(() => {
      if (!basics.value?.HP) return '1 (1d8)';
      const HP = basics.value.HP;
      const dieType = HP.overrideDie ?? HP.defaultDie ?? 8;
      const modBonus = renderBonus(HP.HPmodifier);
      return `${calculatedAvgHp.value} (${HP.HDAmount}d${dieType}${modBonus})`;
  });
  
  const statsWithModifiers = computed(() => {
      const stats = basics.value?.stats || {};
      return Object.keys(statsData.value).map(statKey => { // Iteriere über geladene Stats für korrekte Reihenfolge
          const score = stats[statKey] ?? 10;
          return { stat: statKey, score: score, modifier: statModifier(score) };
      });
  });
  
  const computedPassivePerception = computed(() => {
      if (!monster.value?.senses?.passivePerception) return 10;
      const pp = monster.value.senses.passivePerception;
      if (pp.overrideValue !== null && pp.overrideValue !== undefined) {
          return pp.overrideValue;
      }
      // Neuberechnung (benötigt Skill-Bonus-Logik)
      const wisMod = statModifier(basics.value?.stats?.WIS);
      const perceptionSkill = monster.value.skills?.find(s => s.skill === 'Perception'); // Skill-Name aus Template
      let skillBonus = wisMod;
      if (perceptionSkill) {
          skillBonus = calculateSkillBonus(perceptionSkill); // Eigene Funktion nötig
      }
      return 10 + skillBonus;
      // return pp.defaultValue ?? 10; // Alternative: Verwende gespeicherten Default
  });
  
  const initiative = computed(() => {
      if (!basics.value?.Initiative) return '+0 (10)';
      const init = basics.value.Initiative;
      if (init.initOverrideValue !== null && init.initOverrideValue !== undefined) {
          const passive = 10 + init.initOverrideValue;
          return `${renderBonus(init.initOverrideValue)} (${passive})`;
      }
      // Neuberechnung des Defaults hier (oder verwende den gespeicherten Default)
      const calculatedDefault = calculateInitiativeBonus(basics.value, init); // Greift auf basics.PB und basics.stats.DEX zu
      const passiveDefault = 10 + calculatedDefault;
      return `${renderBonus(calculatedDefault)} (${passiveDefault})`;
   });
  
  
  // --- Spezifische Berechnungen für Saves, Skills etc. (neu definiert) ---
  function calculateSaveBonus(statKey) {
      if (!monster.value?.saves?.[statKey] || !basics.value?.stats) return 0;
      const saveInfo = monster.value.saves[statKey];
      if (saveInfo.overrideValue !== null && saveInfo.overrideValue !== undefined) {
          return saveInfo.overrideValue;
      }
      const prof = saveInfo.proficient ? proficiencyBonus.value : 0;
      return statModifier(basics.value.stats[statKey]) + prof;
  }
  
  function calculateSkillBonus(skillInfo) {
      if (!skillInfo || !basics.value?.stats) return 0;
       if (skillInfo.overrideValue !== null && skillInfo.overrideValue !== undefined) {
          return skillInfo.overrideValue;
      }
      const skillDefinition = skillsData.value[skillInfo.skill]; // Finde Skill-Definition
      if (!skillDefinition) return 0; // Unbekannter Skill
      const statKey = skillDefinition.stat;
      const profMultiplier = skillInfo.expertise ? 2 : skillInfo.proficient ? 1 : 0;
      const prof = proficiencyBonus.value * profMultiplier;
      return statModifier(basics.value.stats[statKey]) + prof;
  }
  
  // ... (ähnliche Neuberechnungen für Attack-Bonus, Damage-Bonus, Save DCs etc. unter Verwendung von basics.stats, basics.PB) ...
  
  const stats2014 = computed(() => {
       return statsWithModifiers.value.map(s => ({
           ...s,
           renderedModifier: renderBonus(s.modifier)
       }));
  });
  
  const statsAndSavesByKey = computed(() => {
       const data = {};
       if (!staticDataLoaded.value) return data; // Warte auf statische Daten
       statsWithModifiers.value.forEach(s => {
           const renderedSave = renderBonus(calculateSaveBonus(s.stat)); // Verwende neue Funktion
           data[s.stat] = {
               stat: s.stat, score: s.score, modifier: s.modifier,
               renderedModifier: renderBonus(s.modifier), renderedSave,
           };
       });
       return data;
   });
  
  const speeds = computed(() => {
      if (!monster.value?.speeds?.length) return '';
      return monster.value.speeds.map(s => {
        const note = s.note ? ` (${s.note})` : '';
        const type = (s.type && s.type.toLowerCase() !== 'walk') ? ` ${s.type}` : '';
        return `${s.speed ?? 30} ft.${type}${note}`;
      }).join(', ');
  });
  
  const skills = computed(() => {
      if (!monster.value?.skills?.length || !staticDataLoaded.value) return '';
      return monster.value.skills
          .map(s => {
              const label = s.skill; // Name direkt aus Template
              const bonus = renderBonus(calculateSkillBonus(s)); // Verwende neue Funktion
              return `${label} ${bonus}`;
          })
          .join(', ');
  });
  
  const resistances = computed(() => monster.value?.resistances?.join(', ') || '');
  const immunities = computed(() => monster.value?.immunities?.join(', ') || '');
  const vulnerabilities = computed(() => monster.value?.vulnerabilities?.join(', ') || '');
  const conditions = computed(() => monster.value?.conditionImmunities?.join(', ') || ''); // Aus Template
  
  const resistancesLabel = computed(() => is2014.value ? 'Damage Resistances' : 'Resistances');
  const immunitiesLabel = computed(() => is2014.value ? 'Damage Immunities' : 'Immunities');
  const vulnerabilitiesLabel = computed(() => is2014.value ? 'Damage Vulnerabilities' : 'Vulnerabilities');
  
  const immunitiesAndConditions = computed(() => {
      // Verwende conditionImmunities aus dem neuen Template
      const all = [...(monster.value?.immunities ?? []), ...(monster.value?.conditionImmunities ?? [])];
      return all.length > 0 ? all.join(', ') : '';
  });
  
  const senses = computed(() => {
      if (!monster.value?.senses) return '';
      const sensesList = [];
      const s = monster.value.senses;
      if ((s.blindsight ?? 0) > 0) sensesList.push(`blindsight ${s.blindsight} ft.`);
      if ((s.darkvision ?? 0) > 0) sensesList.push(`darkvision ${s.darkvision} ft.`);
      if ((s.tremorsense ?? 0) > 0) sensesList.push(`tremorsense ${s.tremorsense} ft.`);
      if ((s.truesight ?? 0) > 0) sensesList.push(`truesight ${s.truesight} ft.`);
      // Passive Perception und Insight werden jetzt anders berechnet/angezeigt
      sensesList.push(`passive Perception ${calculatePassivePerception()}`); // Eigene Funktion nötig
      sensesList.push(`passive Insight ${calculatePassiveInsight()}`); // Eigene Funktion nötig
      if (s.sensesNotes) {
         sensesList.push(sanitizeWebString(processTokens(s.sensesNotes)));
      }
      return sensesList.join(', ');
  });
  
  // Hilfsfunktionen für passive Werte (benötigen Skill-Bonus-Logik)
  function calculatePassivePerception() {
      if (!monster.value?.senses?.passivePerception) return 10;
      const pp = monster.value.senses.passivePerception;
      if (pp.overrideValue !== null && pp.overrideValue !== undefined) return pp.overrideValue;
      const skillInfo = monster.value.skills?.find(s => s.skill === 'Perception');
      return 10 + calculateSkillBonus(skillInfo);
  }
  function calculatePassiveInsight() {
       if (!monster.value?.senses?.passiveInsight) return 10;
       const pi = monster.value.senses.passiveInsight;
       if (pi.overrideValue !== null && pi.overrideValue !== undefined) return pi.overrideValue;
       const skillInfo = monster.value.skills?.find(s => s.skill === 'Insight');
       return 10 + calculateSkillBonus(skillInfo);
  }
  
  const saves2014 = computed(() => {
      if (!monster.value?.saves || !staticDataLoaded.value) return '';
      const proficientSaves = Object.keys(statsData.value).map(statKey => {
          const saveInfo = monster.value.saves[statKey];
          if (saveInfo?.overrideValue !== null && saveInfo?.overrideValue !== undefined) {
               return `${statKey.toUpperCase()} ${renderBonus(saveInfo.overrideValue)}`;
          } else if (saveInfo?.proficient) {
               const bonus = calculateSaveBonus(statKey); // Verwende neue Funktion
               return `${statKey.toUpperCase()} ${renderBonus(bonus)}`;
          }
          return null;
      }).filter(s => s !== null);
      return proficientSaves.join(', ');
  });
  
  const cr = computed(() => {
      if (!basics.value || typeof basics.value.CR !== 'number' || !staticDataLoaded.value) return '0 (10 XP)';
      const crNumeric = basics.value.CR;
      const crInfo = crData.value.find(c => c.numeric === crNumeric);
      if (!crInfo) return `${crNumeric} (Unknown XP)`;
  
      let xpString = `${crInfo.xp.toLocaleString('en-US')} XP`;
      // Lair CR nicht mehr im Basics-Template
      // if (monster.value.lairCr > -1) { ... }
  
      // PB wird jetzt direkt im Objekt gespeichert, hier nur der String für CR
      const pbString = is2024.value ? `, PB ${renderBonus(crInfo.proficiency)}` : '';
      return `${crInfo.cr} (${xpString}${pbString})`;
  });
  
  const traits = computed(() => { /* ... (processTrait verwenden) ... */ });
  const attacks = computed(() => { /* ... (processAttack verwenden) ... */ });
  const actions = computed(() => { /* ... (processAction verwenden, an neue actions-Struktur anpassen) ... */ });
  const bonusActions = computed(() => { /* ... (processAction verwenden, an neue bonusAction-Struktur anpassen) ... */ });
  const multiattacks = computed(() => { /* ... (processMultiattack verwenden, an neue multiattacks-Struktur anpassen) ... */ });
  const legendaryPreamble = computed(() => { /* ... (processLegendaryPreamble verwenden) ... */ });
  const legendaryActions = computed(() => { /* ... (processLegendaryAction verwenden) ... */ });
  const reactions = computed(() => { /* ... (processReaction verwenden) ... */ });
  const lairActionPreamble = computed(() => { /* ... (processLairActionPreamble verwenden) ... */ });
  const lairActions = computed(() => { /* ... (processTokens für lairActions.description verwenden) ... */ });
  const inventory = computed(() => { /* ... (processTokens verwenden) ... */ });
  
  
  // --- Angepasste Spellcasting Computed Properties ---
  const sanitizedClassSpellcastingPreamble = computed(() => null); // Nicht im neuen Template
  const sanitizedInnateSpellcastingPreamble = computed(() => {
       if (!monster.value?.spellcasting) return '';
       // Passe das Preamble an oder entferne es, da die Struktur anders ist
       return `<b><i>Innate Spellcasting.</b></i> ${monster.value.basics.name}'s innate spellcasting ability is ${getStatFullName(monster.value.spellcasting.stat)} (spell save DC ${monster.value.spellcasting.dc ?? defaultSpellSave(monster.value.spellcasting.stat)})${monster.value.spellcasting.hasAttackrolls ? `, ${renderBonus(monster.value.spellcasting.bonus ?? defaultSpellAttackModifier(monster.value.spellcasting.stat))} to hit with spell attacks` : ''}). ${monster.value.spellcasting.requiresSComponents ? 'It can innately cast the following spells, requiring only somatic components:' : 'It can innately cast the following spells, requiring no material components:' }`;
  });
  
  // === NEU: Synchrone Funktion, die auf die geladenen Spelldaten zugreift ===
  function getResolvedSpellDetails(spellName) { /* ... wie zuvor ... */ }
  // ======================================================================
  
  // === KEINE bekannten Spells by Level mehr im neuen Template ===
  // const resolvedKnownSpellsByLevel = computed(() => { ... });
  // ===============================================================
  
  // === Keine Slots im neuen Template ===
  // const resolvedClassSpellcastingSlots = computed(() => { ... });
  // =========================================
  
  // === Angepasst für neue Innate Struktur ===
  const resolvedInnateSpellcastingLists = computed(() => {
      if (!monster.value?.spellcasting) return [];
      const lists = [];
      const mapping = { atWill: 'At will', once: '1/day each', twice: '2/day each', thrice: '3/day each'};
      for (const key of ['atWill', 'once', 'twice', 'thrice']) {
          const spells = monster.value.spellcasting[key];
          if (spells && spells.length > 0) {
              lists.push({
                  id: key, // Verwende den Key als ID
                  renderedLabel: mapping[key],
                  // Füge Notizen hinzu, falls vorhanden
                  renderedSpells: spells.map(s => `${s.name}${s.note ? ` (${s.note})` : ''}`).join(', ')
              });
          }
      }
      return lists;
  });
  // ============================================
  
  
  // --- Token Processing Funktionen (müssen ggf. angepasst werden) ---
  // Überprüfe alle process* Funktionen, ob sie noch auf die richtigen Monsterdaten zugreifen
  // (z.B. monster.value.basics.stats statt monster.value.stats)
  // und ob die Kontextobjekte (trait, action, attack etc.) der neuen Struktur entsprechen.
  function processTokens(input, context, contextType = 'none', style = '2024') {
      // Ersetze {monster...} durch {basics...} wo nötig
       if (!input || !monster.value?.basics) return input ?? ''; // Braucht basics
       input = input.replace(/\{monster\.name\}/gi, monster.value.basics.name || '');
       // ... weitere Ersetzungen für monster.* Tokens ...
  
       if (!context || contextType === 'none') return processMonsterTokens(input);
       // ... Rest der Funktion ...
  }
  // ... (Rest der process* Funktionen anpassen) ...
  
  
  </script>
  
  <style scoped>
/* Basis-Styles (gemeinsam) */
.statblock {
  color: #000;
  font-family: ff-scala-sans-pro, sans-serif;
  font-size: 1rem;
  letter-spacing: 0.01em;
  padding: 8px;
  width: 100%;
  column-gap: 10px;
  break-inside: avoid;
}
.statblock b { font-family: ff-scala-sans-pro, sans-serif; font-style: normal; font-weight: 700; }
.statblock i { font-family: ff-scala-sans-pro, sans-serif; font-style: italic; }
.statblock b > i, .statblock i > b { font-family: ff-scala-sans-pro, sans-serif; font-style: italic; font-weight: 700; }
.statblock hr { border: 1px solid #9c2b1b; margin: 4px 0; }
.statblock .skill { line-height: 1.2rem; color: #58180d; break-inside: avoid; }
.statblock .skill .name { font-family: ff-scala-sans-pro, ScalySansBold, sans-serif; font-weight: 700; }
.statblock h3.section { font-size: 1.3rem; font-family: ScalaSansCaps, sans-serif; font-weight: 400; letter-spacing: normal; color: #58180d; border-bottom: 1px solid #58180d; margin-bottom: 4px; margin-top: 0.8rem; line-height: 1.5rem; break-after: avoid; }
.statblock .traits, .statblock .attacks, .statblock .actions, .statblock .bonus-actions, .statblock .reactions, .statblock .legendary-actions, .statblock .mythic-actions, .statblock .lair-actions, .statblock .regional-effects, .statblock .innate-spellcasting, .statblock .spellcasting, .statblock .multiattack { break-inside: avoid; margin-bottom: 0.8rem; }
.statblock .trait, .statblock .attack, .statblock .action, .statblock .legendary-actions .preamble, .statblock .mythic-actions .preamble, .statblock .lair-actions .preamble, .statblock .regional-effects .preamble, .statblock .multiattack { line-height: 1.15rem; margin-bottom: 0.8rem; }
.statblock .trait b i, .statblock .attack b i, .statblock .action b i, .statblock .reaction b i, .statblock .legendary b i, .statblock .mythic b i { font-family: ff-scala-sans-pro, ScalySansBoldItalic, sans-serif; font-weight: 700; font-style: italic; }
.statblock .attack i { font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif; font-style: italic; }
.statblock .spell-list { margin-top: 0.4rem; margin-left: 10px; }
.statblock .spell-list .spell-row { margin-left: 20px; text-indent: -20px; margin-bottom: 0.2rem; }
.statblock .spell-list .spell-label { font-weight: bold; }
.statblock .spell-list .spell-list-entries { font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif; font-style: italic; }
.statblock .legendary.action, .statblock .action.legendary { margin-bottom: 0.4rem; margin-left: 20px; text-indent: -20px; line-height: 1.15rem; }
.statblock .lair-actions ul, .statblock .regional-effects ul { list-style: disc; margin-left: 30px; padding-left: 0; }
.statblock .lair-actions li, .statblock .regional-effects li { margin-bottom: 0.5rem; line-height: 1.15rem; }

/* --- 2014 Specific Styles --- */
.statblock.mm2014 {
  background-color: #fdf1dc; /* Heller Hintergrund */
  border: none; /* Kein expliziter Border wie 2024 */
  outline: none;
  padding: 8px; /* Original Padding */
}
.statblock.mm2014 .monster-name {
   font-family: mrs-eaves-roman-small-caps, MrEaves, serif; /* Spezielle Schrift */
   color: #58180d;
   font-weight: 800;
   font-size: 2.2em; /* Größer als 2024 */
   margin: 0;
   line-height: 2.5rem;
   border-bottom: none; /* Kein Border unter dem Namen */
}
.statblock.mm2014 .type {
   font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif; /* Kursiv */
   font-style: italic;
   color: #000; /* Schwarz statt grau */
}
/* Keine Einrückung für normale Skills in 2014 */
.statblock.mm2014 .skill { padding-left: 0; text-indent: 0; }
.statblock.mm2014 .row.stats-2014 { display: flex; /* Flexbox für einfache Reihe */ flex-wrap: nowrap; }
.statblock.mm2014 .stat-container { display: flex; flex-grow: 1; flex-direction: column; justify-content: center; align-items: center; color: #58180d; }
.statblock.mm2014 .stat-container .stat-name { font-family: ff-scala-sans-pro, ScalySansBold, sans-serif; font-weight: 700; }
.statblock.mm2014 .stat-container .stat { display: flex; font-size: 0.95rem; }
.statblock.mm2014 .stat-container .stat .score { margin-right: 4px; }
.statblock.mm2014 .inventory { /* Styling für das untere Inventar */
    break-inside: avoid; margin-bottom: 0.8rem;
}
.statblock.mm2014 .attack i:first-of-type { /* Kursiv für Melee/Ranged Attack */
    font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif;
    font-style: italic;
}


/* --- 2024 Specific Styles --- */
.statblock.mm2024 {
  background-color: #e4e2d9;
  border-radius: 12px;
  border: 2px solid #69665f;
  outline: 2px solid #69665f;
  outline-offset: -6px;
  padding: 10px;
}
.statblock.mm2024 .monster-name {
  font-family: ScalaSansCaps, sans-serif;
  letter-spacing: -1px;
  font-size: 1.8em;
  border-bottom: 1px solid #58180d;
  line-height: 2rem;
}
.statblock.mm2024 .type { color: #69665f; }
.statblock.mm2024 hr { margin: 0.5rem 0; border: none; border-top: 1px solid #9c2b1b; /* Linie für 2024 anpassen */}
.statblock.mm2024 .skill { padding-left: 18px; text-indent: -18px; } /* Einrückung */
.statblock.mm2024 .ac { display: grid; grid-template-columns: 1fr auto; grid-template-rows: auto; align-items: baseline; gap: 10px; padding-left: 0; text-indent: 0; }
.statblock.mm2024 .ac span b { font-weight: bold; }
.statblock.mm2024 .stats { display: grid; grid-template-columns: repeat(3, 1fr); grid-template-rows: auto; gap: 10px; margin-bottom: 1em; }
.statblock.mm2024 .stats .stat-table { display: grid; grid-template-columns: repeat(4, 1fr); grid-template-rows: repeat(3, auto); grid-template-areas: 'blank blank mod save' 'stat-1 score-1 mod-1 save-1' 'stat-2 score-2 mod-2 save-2'; }
.statblock.mm2024 .stats .stat-table div { display: flex; align-items: center; justify-content: center; color: #58180d; font-size: 0.9rem; padding: 2px 0; min-height: 1.5em; }
.statblock.mm2024 .stats .header-label { font-family: ScalaSansCaps, sans-serif; font-size: 11px; color: #69665f; text-transform: uppercase; font-weight: bold; }
.statblock.mm2024 .stats .header-label.mod { grid-area: mod; }
.statblock.mm2024 .stats .header-label.save { grid-area: save; }
.statblock.mm2024 .stats .stat { font-weight: bold; font-family: ScalaSansCaps, sans-serif; text-transform: uppercase; font-size: 0.85rem; text-align: left; padding-left: 5px; justify-content: flex-start; }
.statblock.mm2024 .stats .stat.one, .statblock.mm2024 .stats .score.one { background-color: #dcd2ba; }
.statblock.mm2024 .stats .stat.two, .statblock.mm2024 .stats .score.two { background-color: #d0d5b5; }
.statblock.mm2024 .stats .mod.one, .statblock.mm2024 .stats .save.one { background-color: #d7c9c2; }
.statblock.mm2024 .stats .mod.two, .statblock.mm2024 .stats .save.two { background-color: #d4c8db; }
.statblock.mm2024 .stats .stat.one { grid-area: stat-1; }
.statblock.mm2024 .stats .score.one { grid-area: score-1; }
.statblock.mm2024 .stats .mod.one { grid-area: mod-1; }
.statblock.mm2024 .stats .save.one { grid-area: save-1; }
.statblock.mm2024 .stats .stat.two { grid-area: stat-2; }
.statblock.mm2024 .stats .score.two { grid-area: score-2; }
.statblock.mm2024 .stats .mod.two { grid-area: mod-2; }
.statblock.mm2024 .stats .save.two { grid-area: save-2; }
.statblock.mm2024 h3.first.section { margin-top: 0.5em; }

</style>
  