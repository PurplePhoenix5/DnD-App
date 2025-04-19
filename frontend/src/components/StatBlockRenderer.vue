<template>
  <!-- Zeige Ladezustand für Spells -->
  <div v-if="isLoadingSpells" class="text-center pa-4">
    <v-progress-circular indeterminate color="primary"></v-progress-circular>
    <p class="mt-2 text-caption">Loading spell data...</p>
  </div>
  <!-- Zeige den Statblock, wenn Spells geladen sind und Monsterdaten existieren -->
  <div v-else-if="monster" id="render" class="statblock mm2024" :style="columnStyle">
    <!-- Name -->
    <h2 class="monster-name">{{ monster.name }}</h2>
    <!-- Type, Size, Alignment -->
    <div class="type">
      {{ monster.size }} {{ monster.type || 'unknown type' }}, {{ monster.alignment || 'unaligned' }}
    </div>
    <hr />
    <!-- AC, Initiative -->
    <div class="skill ac">
      <div>
        <span class="name">Armor Class</span> {{ monster.AC ?? 10 }}{{ monster.ACType ? ` (${monster.ACType})` : '' }}
      </div>
      <span v-if="initiative"><b>Initiative</b> {{ initiative }}</span>
    </div>
    <!-- HP -->
    <div class="skill"><span class="name">Hit Points</span> {{ hp }}</div>
    <!-- Speed -->
    <div class="skill"><span class="name">Speed</span> {{ speeds }}</div>
    <hr />
    <!-- Stats Table (2024 Style) -->
    <div class="row no-wrap stats" style="width: 100%">
      <div class="stat-table">
        <div class="header-label mod">Mod</div>
        <div class="header-label save">Save</div>
        <div class="stat one">STR</div>
        <div class="score one">{{ statsAndSavesByKey.STR?.score }}</div>
        <div class="mod one">{{ statsAndSavesByKey.STR?.renderedModifier }}</div>
        <div class="save one">{{ statsAndSavesByKey.STR?.renderedSave }}</div>
        <div class="stat two">INT</div>
        <div class="score two">{{ statsAndSavesByKey.INT?.score }}</div>
        <div class="mod two">{{ statsAndSavesByKey.INT?.renderedModifier }}</div>
        <div class="save two">{{ statsAndSavesByKey.INT?.renderedSave }}</div>
      </div>
      <div class="stat-table">
        <div class="header-label mod">Mod</div>
        <div class="header-label save">Save</div>
        <div class="stat one">DEX</div>
        <div class="score one">{{ statsAndSavesByKey.DEX?.score }}</div>
        <div class="mod one">{{ statsAndSavesByKey.DEX?.renderedModifier }}</div>
        <div class="save one">{{ statsAndSavesByKey.DEX?.renderedSave }}</div>
        <div class="stat two">WIS</div>
        <div class="score two">{{ statsAndSavesByKey.WIS?.score }}</div>
        <div class="mod two">{{ statsAndSavesByKey.WIS?.renderedModifier }}</div>
        <div class="save two">{{ statsAndSavesByKey.WIS?.renderedSave }}</div>
      </div>
      <div class="stat-table">
        <div class="header-label mod">Mod</div>
        <div class="header-label save">Save</div>
        <div class="stat one">CON</div>
        <div class="score one">{{ statsAndSavesByKey.CON?.score }}</div>
        <div class="mod one">{{ statsAndSavesByKey.CON?.renderedModifier }}</div>
        <div class="save one">{{ statsAndSavesByKey.CON?.renderedSave }}</div>
        <div class="stat two">CHA</div>
        <div class="score two">{{ statsAndSavesByKey.CHA?.score }}</div>
        <div class="mod two">{{ statsAndSavesByKey.CHA?.renderedModifier }}</div>
        <div class="save two">{{ statsAndSavesByKey.CHA?.renderedSave }}</div>
      </div>
    </div>
    <hr />
    <!-- Skills -->
    <div v-show="skills" class="skill">
      <span class="name">Skills</span> {{ skills }}
    </div>
    <!-- Gear -->
    <div v-show="inventory" class="skill">
      <span class="name">Gear</span> <span v-html="inventory"></span>
    </div>
    <!-- Resistances -->
    <div v-show="resistances" class="skill">
      <span class="name">Damage Resistances</span> {{ resistances }}
    </div>
    <!-- Immunities (Damage + Condition) -->
    <div v-show="immunitiesAndConditions" class="skill">
      <span class="name">Damage & Condition Immunities</span> {{ immunitiesAndConditions }}
    </div>
    <!-- Vulnerabilities -->
    <div v-show="vulnerabilities" class="skill">
      <span class="name">Damage Vulnerabilities</span> {{ vulnerabilities }}
    </div>
    <!-- Senses -->
    <div v-show="senses" class="skill">
      <span class="name">Senses</span> {{ senses }}
    </div>
    <!-- Languages -->
    <div class="skill">
      <span class="name">Languages</span> {{ monster.languages || '—' }}
    </div>
    <!-- Challenge -->
    <div class="skill">
      <span class="name">Challenge</span> {{ cr }}
    </div>
    <hr />

    <!-- Traits -->
    <template v-if="traits && traits.length > 0">
      <h3 class="section first">Traits</h3>
      <div class="traits">
        <div v-for="(trait, idx) in traits" :key="`trait-${idx}`" class="trait" v-html="trait"></div>
        <div v-if="mythicTrait" class="trait" v-html="mythicTrait"></div>
      </div>
    </template>

    <!-- Actions -->
    <h3 class="section">Actions</h3>
    <div v-if="multiattacks" class="multiattack" v-html="multiattacks"></div>
    <div v-for="(attack, idx) in attacks" :key="`attack-${idx}`" class="attack" v-html="attack"></div>
    <div v-for="(action, idx) in actions" :key="`action-${idx}`" class="action" v-html="action"></div>

    <!-- Innate Spellcasting -->
     <div v-if="resolvedInnateSpellcastingLists && resolvedInnateSpellcastingLists.length > 0" class="innate-spellcasting">
       <div v-html="sanitizedInnateSpellcastingPreamble"></div>
       <div class="spell-list">
         <!-- Verwende resolvedInnateSpellcastingLists -->
         <div v-for="innate in resolvedInnateSpellcastingLists" :key="innate.id" class="spell-row">
           <span class="spell-label">{{ innate.renderedLabel }}: </span>
           <span class="spell-list-entries">{{ innate.renderedSpells }}</span>
         </div>
       </div>
     </div>

    <!-- Class Spellcasting -->
    <!-- Verwende die neuen resolved* computed properties -->
    <div v-if="(resolvedClassSpellcastingSlots && resolvedClassSpellcastingSlots.length > 0) || (resolvedKnownSpellsByLevel[0] && resolvedKnownSpellsByLevel[0].length > 0)" class="spellcasting">
      <div v-html="sanitizedClassSpellcastingPreamble"></div>
      <div class="spell-list">
        <!-- Cantrips -->
        <!-- Verwende resolvedKnownSpellsByLevel -->
        <div v-if="resolvedKnownSpellsByLevel[0] && resolvedKnownSpellsByLevel[0].length > 0" class="spell-row">
          <span class="spell-label">Cantrips (at will): </span>
          <span class="spell-list-entries">{{ resolvedKnownSpellsByLevel[0].join(', ') }}</span>
        </div>
        <!-- Warlock Slots -->
        <div v-if="monster.spellcasting?.class === 'WARLOCK' && classSpellcastingWarlockLabel">
          <div class="spell-row">
            <span class="spell-label">{{ classSpellcastingWarlockLabel }}: </span>
             <!-- Warlock verwendet immer noch monster.spellcasting.standard direkt -->
            <span class="spell-list-entries">{{ monster.spellcasting.standard.join(', ') }}</span>
          </div>
        </div>
        <!-- Standard Slots -->
        <template v-else>
           <!-- Verwende resolvedClassSpellcastingSlots -->
          <div v-for="slot in resolvedClassSpellcastingSlots" :key="`slot-${slot.level}`" class="spell-row">
            <span class="spell-label">{{ slot.renderedLabel }}</span>
            <span class="spell-list-entries">{{ slot.renderedSpells }}</span>
          </div>
        </template>
      </div>
    </div>

    <!-- Bonus Actions -->
    <template v-if="bonusActions && bonusActions.length > 0">
      <h3 class="section">Bonus Actions</h3>
      <div class="bonus-actions">
        <div v-for="(action, idx) in bonusActions" :key="`bonus-${idx}`" class="action" v-html="action"></div>
      </div>
    </template>

    <!-- Legendary Actions -->
    <template v-if="legendaryActions && legendaryActions.length > 0">
       <h3 class="section">Legendary Actions</h3>
       <div class="legendary-actions">
         <div class="preamble" v-html="legendaryPreamble"></div>
         <div v-for="(action, idx) in legendaryActions" :key="`legendary-${idx}`" class="action legendary" v-html="action"></div>
       </div>
    </template>

    <!-- Mythic Actions -->
    <template v-if="mythicActions && mythicActions.length > 0">
      <h3 class="section">Mythic Actions</h3>
      <div class="mythic-actions">
        <div class="preamble" v-html="mythicPreamble"></div>
        <div v-for="(action, idx) in mythicActions" :key="`mythic-${idx}`" class="action legendary" v-html="action"></div>
       </div>
    </template>

    <!-- Reactions -->
    <template v-if="reactions && reactions.length > 0">
       <h3 class="section">Reactions</h3>
       <div class="reactions">
         <div v-for="(reaction, idx) in reactions" :key="`reaction-${idx}`" class="action reaction" v-html="reaction"></div>
       </div>
    </template>

    <!-- Lair Actions -->
    <template v-if="lairActions && lairActions.length > 0">
      <h3 class="section">Lair Actions</h3>
      <div class="lair-actions">
        <div class="preamble" v-html="lairActionPreamble"></div>
        <ul>
          <li v-for="(lairAction, idx) in lairActions" :key="`lair-${idx}`" class="action lair" v-html="lairAction"></li>
        </ul>
      </div>
    </template>

    <!-- Regional Effects -->
    <template v-if="regionalEffects && regionalEffects.length > 0">
       <h3 class="section">Regional Effects</h3>
       <div class="regional-effects">
         <div class="preamble" v-html="regionalEffectPreamble"></div>
         <ul>
           <li v-for="(effect, idx) in regionalEffects" :key="`regional-${idx}`" class="action regional" v-html="effect"></li>
         </ul>
       </div>
    </template>

  </div>
   <!-- Fallback, wenn Monsterdaten fehlen -->
  <div v-else>
    Loading monster data or monster data is missing...
  </div>
</template>
  
<script setup>
// Importiere watch, ref, reactive zusätzlich
import { computed, defineProps, unref, watch, ref, reactive } from 'vue';
import _ from 'lodash';
import N2W from 'number-to-words';

// Importiere Daten und Math-Funktionen
import { CR, getCrByNumber, getCrByString } from '../utils/dndData/crData.js';
import { SKILLS, SKILL_KEYS } from '../utils/dndData/skills.js';
import { STATS_FULL, STAT_KEYS } from '../utils/dndData/stats.js';
import { SRD_CLASSES, ClassCastingStat, ClassSpellSlots } from '../utils/dndData/classes.js';
import { ATTACK_RANGES, ATTACK_KINDS } from '../utils/dndData/attackData.js';
import { getRechargeLabel } from '../utils/dndData/rechargeTimes.js';
// Importiere die ASYNCHRONE getSpellData Funktion
import { getSpellData } from '../utils/spellsData.js'; // Passe ggf. Pfad an

// Import rendering functions
import {
    renderBonus, statModifier, avgRoll, avgHP,
    saveModifierForStat, bonusForSkill, saveForAction,
    bonusForAttack, bonusForAttackDamage, bonusForConditionalDamage
} from '../utils/mathRendering.js';

// --- Props ---
const props = defineProps({
    monsterData: { type: Object, required: true, default: () => ({}) },
    columns: { type: Number, default: 1, }
});

const monster = computed(() => props.monsterData);

// === NEU: Zustand für geladene Spelldaten und Ladeanzeige ===
const isLoadingSpells = ref(false);
// Speichert die Detaildaten der relevanten Spells { spellNameLowercase: spellData }
const resolvedSpellDetails = ref({});
// ============================================================

const columnStyle = computed(() => ({ columnCount: props.columns }));

// === NEU: Asynchrone Funktion zum Laden der benötigten Spelldaten ===
async function fetchRelevantSpellData(spellcastingData) {
    // Wenn keine Spellcasting-Daten vorhanden oder leer, nichts tun
    if (!spellcastingData || (!spellcastingData.standard?.length && !spellcastingData.atWill?.length)) {
        resolvedSpellDetails.value = {}; // Leeren
        isLoadingSpells.value = false; // Sicherstellen, dass Ladezustand beendet ist
        return;
    }

    isLoadingSpells.value = true;
    const neededSpellNames = new Set();

    // Sammle alle Spellnamen
    (spellcastingData.standard || []).forEach(name => neededSpellNames.add(name));
    (spellcastingData.atWill || []).forEach(atWillEntry => {
        (atWillEntry.spells || []).forEach(name => neededSpellNames.add(name));
    });

    // Wenn nach dem Sammeln keine Spells vorhanden, beende Laden
    if (neededSpellNames.size === 0) {
        resolvedSpellDetails.value = {};
        isLoadingSpells.value = false;
        return;
    }

    console.log(`StatBlockRenderer: Fetching details for ${neededSpellNames.size} spells...`);

    // Lade Daten für jeden benötigten Spell
    const spellPromises = Array.from(neededSpellNames).map(async (name) => {
         try {
             // Rufe die async Funktion auf (die jetzt fetch verwendet)
             const spell = await getSpellData(name);
             // Gib Name und Daten zurück (oder null bei Fehler in getSpellData)
             return { name, data: spell };
         } catch (error) {
             // Dieser Catch sollte eigentlich nicht nötig sein, wenn getSpellData Fehler intern behandelt
             console.error(`Unexpected error in getSpellData for "${name}":`, error);
             return { name, data: null };
         }
    });

    try {
        const results = await Promise.all(spellPromises);
        const detailsMap = {};
        let successfulFetches = 0;
        results.forEach(result => {
            if (result.data && result.data.name) { // Prüfe, ob Laden erfolgreich war
                detailsMap[result.data.name.toLowerCase()] = result.data;
                successfulFetches++;
            } else {
                 console.warn(`StatBlockRenderer: Could not resolve spell details for "${result.name}".`);
            }
        });
        resolvedSpellDetails.value = detailsMap;
        console.log(`StatBlockRenderer: Successfully loaded details for ${successfulFetches}/${neededSpellNames.size} spells.`);
    } catch (error) {
        // Dieser Catch ist für Fehler in Promise.all selbst (sehr unwahrscheinlich)
        console.error("StatBlockRenderer: Error fetching relevant spell data batch:", error);
        resolvedSpellDetails.value = {};
    } finally {
        isLoadingSpells.value = false;
    }
}
// ============================================================

// === NEU: Watcher, der das Laden anstößt ===
watch(() => props.monsterData, (newMonsterData, oldMonsterData) => {
    // Rufe fetchRelevantSpellData auf, wenn sich monsterData ändert
    // oder wenn es initial geladen wird.
    // Prüfe, ob sich der Spellcasting-Teil tatsächlich geändert hat, um unnötige Loads zu vermeiden (optional)
    if (!_.isEqual(newMonsterData?.spellcasting, oldMonsterData?.spellcasting)) {
        fetchRelevantSpellData(newMonsterData?.spellcasting);
    }
}, { immediate: true, deep: true }); // immediate: true für initiales Laden, deep: true für Änderungen im Objekt
  
  // --- Replicated Monster Store Getters/Logic ---
  
  const proficiencyBonus = computed(() => monster.value?.proficiency ?? 2);
  
  const calculatedAvgHp = computed(() => {
      if (!monster.value?.HP) return 1;
      // Use the (placeholder) avgHP function if available, otherwise calculate manually
      return typeof avgHP === 'function'
          ? avgHP(monster.value.HP)
          : Math.max(1, avgRoll(monster.value.HP.HD ?? 1, monster.value.HP.type ?? 8) + (monster.value.HP.modifier ?? 0));
  });
  
  const statsWithModifiers = computed(() => {
      return STAT_KEYS.map(statKey => {
          const score = monster.value?.stats?.[statKey] ?? 10;
          return {
              stat: statKey,
              score: score,
              modifier: statModifier(score),
          };
      });
  });
  
  const computedPassivePerception = computed(() => {
      if (!monster.value) return 10;
      if (monster.value.passivePerception?.override) {
          return monster.value.passivePerception.overrideValue ?? 10;
      }
      // Find perception skill info *from the monster object*
      const perceptionSkillInstance = monster.value.skills?.find(s => s.key === 'PERCEPTION');
      let perceptionBonus;
  
      if (perceptionSkillInstance) {
          if (perceptionSkillInstance.override) {
              perceptionBonus = perceptionSkillInstance.overrideValue ?? 0;
          } else {
               // Pass the instance from the monster, not the generic definition
              perceptionBonus = bonusForSkill(monster.value, perceptionSkillInstance);
          }
      } else {
           // If skill not present, calculate from raw WIS mod
           perceptionBonus = statModifier(monster.value.stats?.WIS ?? 10);
      }
      return 10 + perceptionBonus;
  });
  
  function defaultSpellSave(statKey) {
      if (!monster.value || !statKey || statKey === 'none' || !monster.value.stats?.[statKey]) {
          return 8 + proficiencyBonus.value + 0; // Fallback DC
      }
      const score = monster.value.stats[statKey];
      return 8 + proficiencyBonus.value + statModifier(score);
  }
  
  function defaultSpellAttackModifier(statKey) {
      if (!monster.value || !statKey || statKey === 'none' || !monster.value.stats?.[statKey]) {
          return renderBonus(proficiencyBonus.value); // Fallback
      }
      const score = monster.value.stats[statKey];
      return renderBonus(proficiencyBonus.value + statModifier(score));
  }
  
  const spellcastingStat = computed(() => monster.value?.spellcasting?.stat ?? 'INT');
  
  const spellAbilityModifier = computed(() => {
      if (!monster.value?.spellcasting) return renderBonus(0);
      if (monster.value.spellcasting.modifier?.override) {
          return renderBonus(monster.value.spellcasting.modifier.overrideValue);
      }
      const score = monster.value.stats?.[spellcastingStat.value] ?? 10;
      return renderBonus(statModifier(score));
  });
  
  const spellSave = computed(() => {
      if (!monster.value?.spellcasting) return 10; // Default DC
      if (monster.value.spellcasting.save?.override) {
          return monster.value.spellcasting.save.overrideValue;
      }
      return defaultSpellSave(spellcastingStat.value);
  });
  
  const spellAttackModifier = computed(() => {
      if (!monster.value?.spellcasting) return '+0';
      if (monster.value.spellcasting.attack?.override) {
          return renderBonus(monster.value.spellcasting.attack.overrideValue);
      }
      return defaultSpellAttackModifier(spellcastingStat.value);
  });
  
  function attackModifier(attackId) {
      const attack = monster.value?.attacks?.find(a => a.id === attackId);
      if (!attack) return renderBonus(0);
      // Use placeholder bonusForAttack
      const bonus = bonusForAttack(monster.value, attack);
      return renderBonus(bonus);
  }
  
  function attackDamageModifier(attackId) {
       const attack = monster.value?.attacks?.find(a => a.id === attackId);
       if (!attack) return 0;
       // Use placeholder bonusForAttackDamage
       return bonusForAttackDamage(monster.value, attack);
  }
  
  function conditionalDamageModifier(attackId) {
      const attack = monster.value?.attacks?.find(a => a.id === attackId);
      if (!attack) return 0;
       // Use placeholder bonusForConditionalDamage
      return bonusForConditionalDamage(monster.value, attack);
  }
  
  function attackName(attackId) {
      return monster.value?.attacks?.find(a => a.id === attackId)?.name ?? `Attack (${attackId})`;
  }
  
  function actionName(actionId) {
      return monster.value?.actions?.find(a => a.id === actionId)?.name ?? `Action (${actionId})`;
  }
  
  function legendaryAction(actionId) {
       const action = monster.value?.actions?.find(a => a.id === actionId);
       if (action) return { type: 'action', action: action };
       const attack = monster.value?.attacks?.find(a => a.id === actionId);
       // Treat attack as action for simplicity here
       if (attack) return { type: 'attack', action: attack };
       return null;
  }
  
  // --- Token Processing (Adapted from useProcessTokens) ---
  
  function listJoin(list, sep = ', ', conjunction = 'and') {
      if (!list || list.length === 0) return '';
      if (list.length === 1) return list[0];
      const part1 = list.slice(0, list.length - 1).join(sep);
      return `${part1}${list.length > 1 ? ` ${conjunction} ` : ''}${list[list.length - 1]}`;
  }
  
  function processMonsterTokens(input) {
      if (!input || !monster.value) return input ?? '';
  
      // HP: {monster.hp} -> {HD}d{Type}[+/-Mod] -> avg (XdY[+/-Mod])
      input = input.replace(/\{monster.hp\}/gi, () => `{${monster.value.HP?.HD ?? 1}d${monster.value.HP?.type ?? 8}${renderBonus(monster.value.HP?.modifier ?? 0)}}`);
  
      // Dice: {xdy[+/-]z}
      const diceRegex = /\{(\d+)d(\d+)\s*([+-]\s*\d+)?\}/gi;
      input = input.replace(diceRegex, (match, count, dice, modifierStr) => {
          // Use placeholder avgRoll and renderBonus
          const countVal = parseInt(count);
          const diceVal = parseInt(dice);
          const modifierVal = modifierStr ? parseInt(modifierStr.replace(/\s/g, '')) : 0;
          if (isNaN(countVal) || isNaN(diceVal) || countVal <= 0 || diceVal <= 0) return match;
          const avg = avgRoll(countVal, diceVal) + modifierVal;
          if (diceVal === 1) return `${avg}`;
          return `${avg} (${countVal}d${diceVal}${renderBonus(modifierVal)})`;
      });
  
      // Saves: {DC:STAT} -> DC X
      const saveRegex = /\{DC:(\w{3})\}/gi;
      input = input.replace(saveRegex, (match, stat) => {
          const upperStat = stat.toUpperCase();
          return STAT_KEYS.includes(upperStat) ? `DC ${defaultSpellSave(upperStat)}` : match;
      });
  
      // Attack Mod: {A:STAT} -> +X
      const attackRegex = /\{A:(\w{3})\}/gi;
      input = input.replace(attackRegex, (match, stat) => {
          const upperStat = stat.toUpperCase();
          return STAT_KEYS.includes(upperStat) ? defaultSpellAttackModifier(upperStat) : match;
      });
  
      // Name: {NAME} -> (The) Nickname/Name
      const nameToken = monster.value.useArticleInToken ? 'the ' : '';
      const displayName = monster.value.nickname || monster.value.name || 'the creature';
      input = input.replace(/\{NAME\}/gi, `${nameToken}${displayName}`);
      // Capitalize 'The' at start of sentence/after period
      input = input.replace(/(?:\.\s*|^)\s*(?:<\/?[b|i]>)*\s*(the)/gm, (match) => match.replace('the', 'The'));
  
  
      // XP: {XP:CR|monster[+]} -> X XP
      const xpRegex = /\{XP:([\d/]+|monster)(\+)?\}/gi;
      input = input.replace(xpRegex, (match, crStr, additive) => {
          const lookupCrStr = crStr.toLowerCase() === 'monster' ? monster.value.CR : crStr;
          // Use getCrByNumber for numeric CR, getCrByString otherwise
          const crData = typeof lookupCrStr === 'number' ? getCrByNumber(lookupCrStr) : getCrByString(lookupCrStr);
  
          if (crData) {
              if (additive) {
                  const monsterCrData = getCrByNumber(monster.value.CR);
                  return `${(crData.xp + (monsterCrData?.xp ?? 0)).toLocaleString('en-US')} XP`;
              } else {
                  return `${crData.xp.toLocaleString('en-US')} XP`;
              }
          }
          return match;
      });
  
      // Generic: {monster.path.to.prop}
      const genericRegex = /\{monster.([\w\d\[\].]+)\}/gi;
      input = input.replace(genericRegex, (match, prop) => {
          const value = _.get(monster.value, prop);
          return value !== undefined ? String(value) : match;
      });
  
      return input;
  }
  
  function processContextTokens(input, context, contextType) {
       if (!input || !monster.value) return input ?? '';
       if (!context || contextType === 'none') return processMonsterTokens(input); // Only process monster tokens if no context
  
       // Context-specific tokens
       if (contextType === 'trait' && context.limitedUse) {
           const limitedUseText = context.limitedUse.count > 0 ? ` (${context.limitedUse.count}/${getRechargeLabel(context.limitedUse.rate)})` : '';
           input = input.replace(/\{trait.limitedUse\}/gi, limitedUseText);
           input = input.replace(/\{trait.limitedUse.rate\}/gi, getRechargeLabel(context.limitedUse.rate));
       } else if (contextType === 'action' && (context.limitedUse || context.recharge)) {
           let limitedUseText = '';
           if (context.recharge) limitedUseText = ` (Recharge ${context.recharge})`;
           else if (context.limitedUse?.count > 0) limitedUseText = ` (${context.limitedUse.count}/${getRechargeLabel(context.limitedUse.rate)})`;
           input = input.replace(/\{action.limitedUse\}/gi, limitedUseText);
           if(context.limitedUse) input = input.replace(/\{action.limitedUse.rate\}/gi, getRechargeLabel(context.limitedUse.rate));
           // Action Effects
           const effectsText = context.effects?.map(e => `<i>${e.case}:</i> ${e.effect}`).join(' ') ?? '';
           input = input.replace(/\{action.effects\}/gi, effectsText);
       } else if (contextType === 'reaction' && context.limitedUse) {
           const limitedUseText = context.limitedUse.count > 0 ? ` (${context.limitedUse.count}/${getRechargeLabel(context.limitedUse.rate)})` : '';
           input = input.replace(/\{reaction.limitedUse\}/gi, limitedUseText);
           input = input.replace(/\{reaction.limitedUse.rate\}/gi, getRechargeLabel(context.limitedUse.rate));
       } else if (contextType === 'spell') {
           input = input.replace(/\{spellcasting.ordinal\}/gi, N2W.toOrdinal(context.level));
           input = input.replace(/\{spellcasting.stat\}/gi, STATS_FULL[context.stat] || context.stat);
           input = input.replace(/\{spellcasting.save\}/gi, `DC ${spellSave.value}`);
           input = input.replace(/\{spellcasting.attack\}/gi, spellAttackModifier.value);
           input = input.replace(/\{spellcasting.ability\}/gi, spellAbilityModifier.value);
           const className = context.class ? (SRD_CLASSES[context.class] || context.class) : '';
           input = input.replace(/\{spellcasting.class\}/gi, className);
       } else if (contextType === 'attack') {
           const distanceText = `(${ATTACK_RANGES[context.distance] || context.distance})`;
           input = input.replace(/\{attack.distance\}/gi, distanceText);
           input = input.replace(/\{attack.modifier\}/gi, attackModifier(context.id));
           let rangeText = '';
           if (context.distance === 'MELEE') rangeText = `reach ${context.range?.reach ?? 5} ft.`;
           else if (context.distance === 'RANGED') rangeText = `range ${context.range?.standard ?? 30}/${context.range?.long ?? 120} ft.`;
           else if (context.distance === 'BOTH') rangeText = `reach ${context.range?.reach ?? 5} ft. or range ${context.range?.standard ?? 30}/${context.range?.long ?? 120} ft.`;
           input = input.replace(/\{attack.range\}/gi, rangeText);
           const targetText = context.targets === 1 ? 'one target' : `${N2W.toWords(context.targets)} targets`;
           input = input.replace(/\{attack.targets\}/gi, targetText);
           // Primary damage - process tokens *within* damage string
           const dmgModVal = attackDamageModifier(context.id);
           const primaryDmgDice = `{${context.damage?.count ?? 1}d${context.damage?.dice ?? 4}${renderBonus(dmgModVal)}} ${context.damage?.type ?? 'bludgeoning'}`;
           input = input.replace(/\{attack.damage\}/gi, processMonsterTokens(primaryDmgDice));
           // Conditional damage
           let conditionalDmgText = '';
           if (context.alternateDamage?.active) {
               const condModVal = conditionalDamageModifier(context.id);
               const condDmgDice = `{${context.alternateDamage.count ?? 1}d${context.alternateDamage.dice ?? 4}${renderBonus(condModVal)}} ${context.alternateDamage.type ?? 'piercing'}`;
               conditionalDmgText = `, or ${processMonsterTokens(condDmgDice)} ${context.alternateDamage.condition ?? ''}`;
           }
           input = input.replace(/\{attack.conditionalDamage\}/gi, conditionalDmgText);
           // Additional damage
           let additionalDmgText = '';
           if (context.additionalDamage?.length > 0) {
               const additionalParts = context.additionalDamage.map(d => {
                   const addDmgDice = `{${d.count ?? 1}d${d.dice ?? 4}} ${d.type ?? 'poison'}`;
                   return `${processMonsterTokens(addDmgDice)}${d.note ? ` (${d.note})` : ''}`;
               });
               additionalDmgText = ` plus ${listJoin(additionalParts, ' plus ')}`;
           }
           input = input.replace(/\{attack.additionalDamage\}/gi, additionalDmgText);
       } else if (contextType === 'multiattack') {
           const renderedMa = (context ?? []).map(ma => {
               const collatedAttacks = {};
               (ma.attacks ?? []).forEach(aId => { collatedAttacks[aId] = (collatedAttacks[aId] || 0) + 1; });
               const attacksText = Object.entries(collatedAttacks).map(([id, count]) => `${N2W.toWords(count)} ${attackName(id)} attack${count > 1 ? 's' : ''}`).join(', ');
  
               const collatedActions = {};
               (ma.actions ?? []).forEach(aId => { collatedActions[aId] = (collatedActions[aId] || 0) + 1; });
               const actionsText = Object.entries(collatedActions).map(([id, count]) => `its ${actionName(id)} ${N2W.toWords(count)} time${count > 1 ? 's' : ''}`).join(', ');
  
               let fullText = '';
               if (actionsText && attacksText) fullText = `uses ${actionsText}, then makes ${attacksText}`;
               else if (actionsText) fullText = `uses ${actionsText}`;
               else if (attacksText) fullText = `makes ${attacksText}`;
               return { attacks: attacksText, actions: actionsText, full: fullText };
           });
           const multiattackAll = `${monster.value.name ?? 'The creature'} ${renderedMa.map(ma => ma.full).join(' or ')}`;
           input = input.replace(/\{multiattack.all\}/gi, multiattackAll);
           input = input.replace(/\{multiattack.postscript\}/gi, monster.value.multiattackOptions?.postscript ?? '');
           // Simplified generic token handling for multiattack
           const genericMaRegex = /\{multiattack.rendered\[\d+]\.(attacks|actions|full)\}/gi;
            input = input.replace(genericMaRegex, (match, prop) => renderedMa[0]?.[prop] ?? match);
       } else if (contextType === 'legendary') {
           const count = context?.count ?? 1;
           input = input.replace(/\{legendaryActions.actions\}/gi, `${count} legendary action${count > 1 ? 's' : ''}`);
       }
  
       // Generic context tokens: {trait.name}, {action.description}, etc.
       const genericContextRegex = new RegExp(`\\{${contextType}\\.([\\w\\d\\[\].]+)\\}`, 'gi');
       input = input.replace(genericContextRegex, (match, prop) => {
           const value = _.get(context, prop);
           return value !== undefined ? String(value) : match;
       });
  
       // Finally, process monster tokens within the result
       return processMonsterTokens(input);
  }
  
  // Main token processing function
  function processTokens(input, context, contextType = 'none') {
      return processContextTokens(input, context, contextType);
  }
  
  // --- Specific Processors (call processTokens) ---
  function processTrait(context) {
      if (!context) return '';
      const template = context.customPreamble ? context.description : `<b><i>{trait.name}{trait.limitedUse}.</i></b> {trait.description}`;
      return processTokens(template, context, 'trait');
  }
  
  function processAction(context) {
    if (!context) return '';
    let template = '';
    if (context.customPreamble) {
        template = context.description;
    } else if (context.stat && context.stat !== 'none') {
        const actionDC = saveForAction(monster.value, context.stat, context.save);
        // === KORREKTUR: Ternary Operator herausgezogen ===
        const rangeText = context.range ? `, ${context.range}` : ''; // Erzeuge den Range-String vorher
        // Baue den Template-String jetzt mit der einfachen Variable zusammen
        template = `<b><i>{action.name}{action.limitedUse}.</i></b> <i>${STATS_FULL[context.stat]} Saving Throw DC ${actionDC}</i>${rangeText}. {action.effects} {action.description}`;
        // ===============================================
    } else {
         template = `<b><i>{action.name}{action.limitedUse}.</i></b> {action.description}`;
    }
     return processTokens(template, context, 'action');
}
  
  function processReaction(context) {
      if (!context) return '';
      const template = context.trigger
          ? `<b><i>${context.name}{reaction.limitedUse}.</i></b> <i>Trigger:</i> ${context.trigger}. <i>Response:</i> ${context.description}`
          : `<b><i>${context.name}{reaction.limitedUse}.</i></b> ${context.description}`;
      return processTokens(template, context, 'reaction');
  }
  
  function processAttack(context) {
    if (!context) return '';
    const template = context.useCustomRenderer
          ? context.customRenderer
          // English preset using tokens replaced by processContextTokens
          : `<b><i>{attack.name}.</i></b> <i>{attack.distance} Weapon Attack:</i> {attack.modifier} to hit, {attack.range}, {attack.targets}. <i>Hit:</i> {attack.damage}{attack.conditionalDamage}{attack.additionalDamage}. {attack.description}`;
    return processTokens(template, context, 'attack');
  }
  
  function processMultiattack(context) {
       if (!monster.value?.multiattacks || monster.value.multiattacks.length === 0) return '';
       const template = monster.value.multiattackOptions?.useCustomRenderer
          ? monster.value.multiattackOptions.customMultiattackRenderer
          // English preset using tokens
          : `<b><i>Multiattack.</i></b> {multiattack.all}. {multiattack.postscript}`;
       return processTokens(template, context, 'multiattack');
  }
  
  function processClassSpellcasting(context) {
      if (!context) return '';
      const template = context.useCustomClassPreamble
          ? context.customClassPreamble
          // English preset using tokens
          : `<b><i>Spellcasting.</b></i> {NAME} is a {spellcasting.ordinal}-level spellcaster. Its spellcasting ability is {spellcasting.stat} (spell save {spellcasting.save}, {spellcasting.attack} to hit with spell attacks). {spellcasting.notes} {NAME} has the following {spellcasting.class} spells prepared:`;
      return processTokens(template, context, 'spell');
  }
  
  function processInnateSpellcasting(context) {
      if (!context) return '';
      const template = context.useCustomInnatePreamble
          ? context.customInnatePreamble
          // English preset (2024 style) using tokens
          : `<b><i>Spellcasting.</b></i> {NAME}'s spellcasting ability is {spellcasting.stat} (spell save {spellcasting.save}). {spellcasting.atWillNotes} It can innately cast the following spells, requiring no material components:`;
      return processTokens(template, context, 'spell');
  }
  
  function processLegendaryPreamble(context) {
    if (!context) return '';
    const template = context.useCustomPreamble
          ? context.customPreamble
          // English preset using tokens
          : `{NAME} can take {legendaryActions.actions}, choosing from the options below. Only one legendary action option can be used at a time and only at the end of another creature's turn. {NAME} regains spent legendary actions at the start of its turn.`;
    return processTokens(template, context, 'legendary');
  }
  
  function processLegendaryAction(actionId, cost) {
      const actionInfo = legendaryAction(actionId);
      if (!actionInfo || !actionInfo.action) return '[Invalid Action or Attack ID]';
  
      const costText = cost > 1 ? ` (Costs ${cost} Actions)` : '';
      let description = '';
  
      if (actionInfo.type === 'action') {
           const action = actionInfo.action;
           if (action.legendaryOnly) {
                // Render full description without preamble if legendaryOnly
                let processedDesc = processAction({ ...action, name: '', limitedUse: null, recharge: '' });
                // Remove the auto-generated bold/italic name part
                description = processedDesc.replace(/^<b><i>\.?<\/i><\/b>\s*/, '');
           } else {
               // Simple reference for standard actions
               description = `{NAME} uses its ${action.name}.`;
           }
      } else if (actionInfo.type === 'attack') {
          // Simple reference for attacks
          description = `{NAME} makes a ${actionInfo.action.name} attack.`;
      }
      // Process monster tokens in the description
      description = processMonsterTokens(description);
  
      return `<b><i>${actionInfo.action.name}${costText}.</i></b> ${description}`;
  }
  
  
  function processMythicActionTrait(context) {
       if (!context) return '';
       // English preset using tokens
       const template = `<b><i>${context.triggerName} (${context.triggerRecharge}).</i></b> ${context.triggerDescription}`;
       return processTokens(template, context, 'mythic');
  }
  
  function processMythicActionPreamble(context) {
       if (!context) return '';
       // English preset using tokens
       const template = context.preamble; // Preamble might contain tokens
       return processTokens(template, context, 'mythic');
  }
  
  function processLairActionPreamble() {
      if (!monster.value) return '';
      const template = monster.value.useCustomLairActionPreamble
          ? monster.value.lairActionPreamble
          // English preset using tokens
          : `When fighting inside its lair, {NAME} can take lair actions. On initiative count 20 (losing initiative ties), {NAME} takes a lair action to cause one of the following effects; {NAME} can't use the same effect two rounds in a row:`;
      return processTokens(template, undefined, 'none');
  }
  
  function sanitizeWebString(input) {
      if (!input) return '';
      input = input.replace(/</g, '&lt;').replace(/>/g, '&gt;');
      input = input.replace(/&lt;(\/?(b|i|u))&gt;/gi, '<$1>'); // Allow b, i, u
      input = input.replace(/&lt;br\s*\/?&gt;/gi, '<br>'); // Allow br
      // Handle specific div structures from editors if necessary
      input = input.replace(/&lt;div&gt;&lt;br&gt;&lt;\/div&gt;/gi, '<div><br></div>');
      input = input.replace(/&lt;(\/?div)&gt;/gi, '<$1>'); // Allow div
      return input;
  }
  
  // === NEU: Synchrone Funktion, die auf die geladenen Spelldaten zugreift ===
function getResolvedSpellDetails(spellName) {
    return resolvedSpellDetails.value[spellName?.toLowerCase()] || null;
}
// ======================================================================

// === NEU: Computed Property, die die Spell-Levels berechnet (synchron) ===
const resolvedKnownSpellsByLevel = computed(() => {
    const spellNames = monster.value?.spellcasting?.standard ?? [];
    const spellsByLevel = {};

    // Cantrips (Level 0) - Immer initialisieren, auch wenn leer
    spellsByLevel[0] = [];
    const atWillCantrips = monster.value?.spellcasting?.atWill
       ?.filter(aw => aw.rate === 'AT_WILL')
       .flatMap(aw => aw.spells) ?? [];
    atWillCantrips.forEach(name => { // Deduplizieren beim Hinzufügen
        if (!spellsByLevel[0].includes(name)) {
           spellsByLevel[0].push(name);
        }
    });


    // Gehe durch die Standardliste und füge basierend auf den geladenen Details hinzu
    spellNames.forEach(name => {
        const spell = getResolvedSpellDetails(name); // Verwende die neue synchrone Funktion
        if (spell) {
            const level = spell.level ?? -1;
             if (level === 0) { // Cantrip in Standardliste
                  if (!spellsByLevel[0].includes(spell.name)) { // Deduplizieren
                     spellsByLevel[0].push(spell.name);
                  }
             } else if (level > 0) { // Normaler Spell
                if (!spellsByLevel[level]) {
                    spellsByLevel[level] = [];
                }
                 if (!spellsByLevel[level].includes(spell.name)) { // Deduplizieren
                     spellsByLevel[level].push(spell.name);
                 }
             }
        } else if (spellNames.includes(name)) {
             // Logge Warnung nur, wenn Spell in Liste, aber Details fehlen
             console.warn(`StatBlockRenderer: Details for spell "${name}" not found or not loaded yet.`);
        }
    });
    return spellsByLevel;
});
// ======================================================================

// === NEU: Computed Property für Slots, verwendet resolvedKnownSpellsByLevel ===
const resolvedClassSpellcastingSlots = computed(() => {
    if (!monster.value?.spellcasting?.slots || monster.value.spellcasting.class === 'WARLOCK') return [];
    const slots = monster.value.spellcasting.slots;
    const resultSlots = [];

    for (let level = 1; level <= 9; level++) {
        const slotCount = slots[level - 1];
        if (slotCount > 0) {
            const spellsAtLevel = resolvedKnownSpellsByLevel.value[level] || []; // Greife auf berechnete Liste zu
            // Zeige Slots auch an, wenn keine Spells bekannt sind (optional, aber oft nützlich)
            // if (spellsAtLevel.length > 0) {
               resultSlots.push({
                    level: level,
                    slots: slotCount,
                    renderedSpells: spellsAtLevel.length > 0 ? spellsAtLevel.join(', ') : '—', // Zeige Strich, wenn keine Spells bekannt
                    renderedLabel: `${N2W.toOrdinal(level)} level (${slotCount} slot${slotCount > 1 ? 's' : ''}): `
                });
            // }
        }
    }
    return resultSlots;
});

  // --- Computed properties for rendering (Adapted from useTextRenderer) ---
  
  const hpModifier = computed(() => renderBonus(monster.value?.HP?.modifier ?? 0));
  const hp = computed(() => {
      if (!monster.value?.HP) return '1 (1d8)';
      return `${calculatedAvgHp.value} (${monster.value.HP.HD ?? 1}d${monster.value.HP.type ?? 8}${hpModifier.value})`;
  });
  
  const statsAndSavesByKey = computed(() => {
       const data = {};
       statsWithModifiers.value.forEach(s => {
           if (!monster.value) return;
           const saveInfo = monster.value.saves?.[s.stat] ?? { proficient: false, override: false };
           // Use placeholder saveModifierForStat
           const renderedSave = saveInfo.override
               ? renderBonus(saveInfo.overrideValue)
               : renderBonus(saveModifierForStat(monster.value, s.stat));
  
           data[s.stat] = {
               stat: s.stat,
               score: s.score,
               modifier: s.modifier,
               renderedModifier: renderBonus(s.modifier),
               renderedSave,
           };
       });
       return data;
   });
  
  
  const speeds = computed(() => {
    if (!monster.value?.speeds) return '';
    return monster.value.speeds.map(s => {
      const note = s.note ? ` (${s.note})` : '';
      const type = (s.type && s.type.toLowerCase() !== 'walk') ? ` ${s.type}` : '';
      return `${s.speed ?? 30} ft.${type}${note}`;
    }).join(', ');
  });
  
  const skills = computed(() => {
      if (!monster.value?.skills) return '';
      return monster.value.skills
          .filter(s => s.key !== 'INITIATIVE') // Hide Initiative
          .map(s => {
              const label = SKILLS[s.key]?.label ?? s.key; // Use English label
              let bonus;
               // Find the instance of the skill on the monster to check override/proficiency
               const skillInstance = monster.value.skills.find(ms => ms.key === s.key);
               if (!skillInstance) return `${label} +0`; // Should not happen if s is from monster.value.skills
  
              if (skillInstance.override) {
                  bonus = renderBonus(skillInstance.overrideValue);
              } else {
                  // Use placeholder bonusForSkill with the skill instance from the monster
                  bonus = renderBonus(bonusForSkill(monster.value, skillInstance));
              }
              return `${label} ${bonus}`;
          })
          .join(', ');
  });
  
  
  const resistances = computed(() => monster.value?.resistances?.join(', ') || '');
  const immunities = computed(() => monster.value?.immunities?.join(', ') || '');
  const vulnerabilities = computed(() => monster.value?.vulnerabilities?.join(', ') || '');
  const conditions = computed(() => monster.value?.conditions?.join(', ') || '');
  
  const immunitiesAndConditions = computed(() => {
      const all = [...(monster.value?.immunities ?? []), ...(monster.value?.conditions ?? [])];
      return all.length > 0 ? all.join(', ') : '';
  });
  
  const senses = computed(() => {
    if (!monster.value?.senses) return '';
    const sensesList = [];
    if ((monster.value.senses.blindsight ?? 0) > 0) sensesList.push(`blindsight ${monster.value.senses.blindsight} ft.`);
    if ((monster.value.senses.darkvision ?? 0) > 0) sensesList.push(`darkvision ${monster.value.senses.darkvision} ft.`);
    if ((monster.value.senses.tremorsense ?? 0) > 0) sensesList.push(`tremorsense ${monster.value.senses.tremorsense} ft.`);
    if ((monster.value.senses.truesight ?? 0) > 0) sensesList.push(`truesight ${monster.value.senses.truesight} ft.`);
  
    sensesList.push(`passive Perception ${computedPassivePerception.value}`);
  
    if (monster.value.sensesNotes) {
       sensesList.push(sanitizeWebString(processTokens(monster.value.sensesNotes, undefined, 'none')));
    }
  
    return sensesList.join(', ');
  });
  
  const cr = computed(() => {
    if (!monster.value) return '0 (10 XP, PB +2)';
    if (monster.value.useCrDisplayOverride) return monster.value.crOverride;
  
    const crData = getCrByNumber(monster.value.CR);
    if (!crData) return '0 (10 XP, PB +2)'; // Fallback
  
    let xpString = `${crData.xp.toLocaleString('en-US')} XP`;
     if (monster.value.lairCr > -1) {
         const lairCrData = getCrByNumber(monster.value.lairCr);
         if (lairCrData) {
              xpString += `, or ${lairCrData.xp.toLocaleString('en-US')} XP ${monster.value.lairCrNote ? `(${monster.value.lairCrNote})` : '(in lair)'}`;
         }
     }
  
    return `${crData.cr} (${xpString}, PB ${renderBonus(crData.proficiency)})`;
  });
  
   const initiative = computed(() => {
     if (!monster.value) return '+0 (10)';
      // Find initiative skill instance on monster
     const initiativeSkill = monster.value.skills?.find(s => s.key === 'INITIATIVE');
     let mod = 0;
     if (initiativeSkill?.override) {
        mod = initiativeSkill.overrideValue ?? 0;
     } else {
        const dexScore = monster.value.stats?.DEX ?? 10;
        mod = statModifier(dexScore);
        // Check proficiency *on the instance*
        if (initiativeSkill?.proficient) mod += proficiencyBonus.value;
     }
     const passive = 10 + mod;
     return `${renderBonus(mod)} (${passive})`;
  });
  
  const traits = computed(() => {
    if (!monster.value?.traits) return [];
    const sortedTraits = monster.value.alphaTraits // Respect sorting flag
      ? _.sortBy(monster.value.traits, 'name')
      : monster.value.traits;
    return sortedTraits.map(trait => sanitizeWebString(processTrait(trait)));
  });
  
  const attacks = computed(() => {
     if (!monster.value?.attacks) return [];
     return monster.value.attacks.map(a => sanitizeWebString(processAttack(a)));
  });
  
  const actions = computed(() => {
     if (!monster.value?.actions) return [];
     return monster.value.actions
       .filter(a => !a.legendaryOnly && !a.bonusAction)
       .map(a => sanitizeWebString(processAction(a)));
  });
  
  const bonusActions = computed(() => {
     if (!monster.value?.actions) return [];
     return monster.value.actions
       .filter(a => !a.legendaryOnly && a.bonusAction)
       .map(a => sanitizeWebString(processAction(a)));
  });
  
  const multiattacks = computed(() => {
     if (!monster.value?.multiattacks || monster.value.multiattacks.length === 0) return '';
     // Pass the actual multiattacks array from the monster
     return sanitizeWebString(processMultiattack(monster.value.multiattacks));
  });
  
  const legendaryPreamble = computed(() => {
     if (!monster.value?.legendaryActions) return '';
     return sanitizeWebString(processLegendaryPreamble(monster.value.legendaryActions));
  });
  
  const legendaryActions = computed(() => {
     if (!monster.value?.legendaryActions?.actions) return [];
     return monster.value.legendaryActions.actions.map(a =>
       sanitizeWebString(processLegendaryAction(a.actionId, a.cost))
     );
  });
  
  const mythicTrait = computed(() => {
      if (!monster.value?.mythicActions?.triggerName) return '';
      return sanitizeWebString(processMythicActionTrait(monster.value.mythicActions));
  });
  
  const mythicPreamble = computed(() => {
     if (!monster.value?.mythicActions?.preamble) return '';
     return sanitizeWebString(processMythicActionPreamble(monster.value.mythicActions));
  });
  
  const mythicActions = computed(() => {
     if (!monster.value?.mythicActions?.actions) return [];
     return monster.value.mythicActions.actions.map(a =>
       sanitizeWebString(processLegendaryAction(a.actionId, a.cost)) // Reuses legendary logic
     );
  });
  
  const reactions = computed(() => {
     if (!monster.value?.reactions) return [];
     return monster.value.reactions.map(r => sanitizeWebString(processReaction(r)));
  });
  
  const lairActionPreamble = computed(() => sanitizeWebString(processLairActionPreamble()));
  
  const lairActions = computed(() => {
     if (!monster.value?.lairActions) return [];
     return monster.value.lairActions.map(la =>
       sanitizeWebString(processTokens(la.description, undefined, 'none'))
     );
  });
  
  const regionalEffectPreamble = computed(() => {
      if (!monster.value?.regionalEffectDescription) return '';
      return sanitizeWebString(processTokens(monster.value.regionalEffectDescription, undefined, 'none'));
  });
  
  const regionalEffects = computed(() => {
      if (!monster.value?.regionalEffects) return [];
      return monster.value.regionalEffects.map(re =>
         sanitizeWebString(processTokens(re.description, undefined, 'none'))
      );
  });
  
  const inventory = computed(() => {
     if (!monster.value?.inventory) return '';
     return sanitizeWebString(processTokens(monster.value.inventory, undefined, 'none'));
  });
  
  // Spellcasting Computed Properties
  const sanitizedClassSpellcastingPreamble = computed(() => {
       if (!monster.value?.spellcasting) return '';
       return sanitizeWebString(processClassSpellcasting(monster.value.spellcasting));
  });
  
  const sanitizedInnateSpellcastingPreamble = computed(() => {
       if (!monster.value?.spellcasting) return '';
       return sanitizeWebString(processInnateSpellcasting(monster.value.spellcasting));
  });
  
   // Function to get spell names for a specific level
   function knownSpellsOfLevel(level) {
       const spellNames = monster.value?.spellcasting?.standard ?? [];
       if (level === 0) {
            // Attempt to get cantrips from atWill if standard list doesn't contain them explicitly
            const atWillCantrips = monster.value?.spellcasting?.atWill
               ?.filter(aw => aw.rate === 'AT_WILL')
               .flatMap(aw => aw.spells) ?? [];
            // You might need additional logic if cantrips are *also* in standard list
            return atWillCantrips;
       }
       // Filter standard spell list based on imported spell data
       return spellNames.filter(name => getSpellData(name)?.level === level);
   }
  
   const classSpellcastingWarlockLabel = computed(() => {
       if (!monster.value?.spellcasting?.slots || monster.value.spellcasting.class !== 'WARLOCK') return '';
       const slots = monster.value.spellcasting.slots;
       for (let idx = 8; idx >= 0; idx--) { // Iterate from 9th level down
         if (slots[idx] > 0) {
           // English Format from original i18n
            return `${N2W.toOrdinal(idx + 1)}-level (${slots[idx]} ${N2W.toOrdinal(idx + 1)}-level slots)`;
         }
       }
       return ''; // Should not happen if Warlock and slots exist
   });
  
    const classSpellcastingSlots = computed(() => {
        if (!monster.value?.spellcasting?.slots || monster.value.spellcasting.class === 'WARLOCK') return [];
  
        const slots = monster.value.spellcasting.slots;
        const resultSlots = [];
  
        for (let level = 1; level <= 9; level++) {
            const slotCount = slots[level - 1];
            if (slotCount > 0) {
                const spellsAtLevel = knownSpellsOfLevel(level);
                if (spellsAtLevel.length > 0) { // Only show slots if spells are known for that level
                   resultSlots.push({
                        level: level,
                        slots: slotCount,
                        renderedSpells: spellsAtLevel.join(', '),
                         // English Format from original i18n
                        renderedLabel: `${N2W.toOrdinal(level)} level (${slotCount} slot${slotCount > 1 ? 's' : ''}): `
                    });
                }
            }
        }
        return resultSlots;
    });
  
    const innateSpellcastingLists = computed(() => {
        if (!monster.value?.spellcasting?.atWill) return [];
        return monster.value.spellcasting.atWill.map(s => ({
            ...s,
            renderedLabel: s.rate === 'AT_WILL'
                            ? 'At will' // English Label
                            : `${s.count}/${getRechargeLabel(s.rate)}`,
            renderedSpells: s.spells.join(', ')
        }));
    });
  
  </script>
  
  <style scoped>
      /* Kopiere die Styles aus meiner vorherigen Antwort hierher */
      /* Basis-Styles */
      .statblock {
        color: #000;
        font-family: ff-scala-sans-pro, sans-serif; /* Fallback font */
        font-size: 1rem; /* Adjust base font size if needed */
        letter-spacing: 0.01em;
        padding: 8px;
        width: 100%;
        column-gap: 10px; /* Gap between columns */
        break-inside: avoid; /* Prevent breaking inside the block */
      }
  
      .statblock b {
        font-family: ff-scala-sans-pro, sans-serif;
        font-style: normal;
        font-weight: 700;
      }
  
      .statblock i {
        font-family: ff-scala-sans-pro, sans-serif;
        font-style: italic;
      }
  
      .statblock b > i,
      .statblock i > b {
        font-family: ff-scala-sans-pro, sans-serif;
        font-style: italic;
        font-weight: 700;
      }
  
      /* 2024 Specific Styles (Adapted as Default) */
      .statblock {
        background-color: #e4e2d9;
        border-radius: 12px;
        border: 2px solid #69665f;
        outline: 2px solid #69665f;
        outline-offset: -6px;
        padding: 10px;
      }
  
      .statblock .monster-name {
        font-family: ScalaSansCaps, sans-serif; /* Fallback */
        letter-spacing: -1px;
        font-size: 1.8em;
        border-bottom: 1px solid #58180d;
        line-height: 2rem;
        color: #58180d;
        margin: 0;
        font-weight: 800;
        break-after: avoid;
      }
  
      .statblock .type {
        color: #69665f;
        margin-top: 2px;
         font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif;
         font-style: italic;
         break-after: avoid;
      }
  
      .statblock hr {
        margin: 0.5rem 0;
        border: 1px solid #9c2b1b;
      }
  
       .statblock .skill {
          line-height: 1.2rem;
          color: #58180d;
          padding-left: 18px; /* Indentation for 2024 */
          text-indent: -18px; /* Indentation for 2024 */
          break-inside: avoid;
       }
  
      .statblock .skill .name {
        font-family: ff-scala-sans-pro, ScalySansBold, sans-serif;
        font-weight: 700;
      }
  
      /* AC / Initiative Line */
      .statblock .ac {
          display: grid;
          grid-template-columns: 1fr auto; /* Auto for Initiative */
          grid-template-rows: auto;
          align-items: baseline;
          gap: 10px;
          padding-left: 0; /* No indent for this line */
          text-indent: 0;
      }
      .statblock .ac span b { /* Initiative Label Bold */
           font-weight: bold;
      }
  
      /* Stats Table */
      .statblock .stats {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          grid-template-rows: auto;
          gap: 10px;
          margin-bottom: 1em;
          break-inside: avoid;
      }
  
      .statblock .stats .stat-table {
          display: grid;
          grid-template-columns: repeat(4, 1fr);
          grid-template-rows: repeat(3, auto);
          grid-template-areas:
              'blank blank mod save'
              'stat-1 score-1 mod-1 save-1'
              'stat-2 score-2 mod-2 save-2';
      }
  
      .statblock .stats .stat-table div {
          display: flex;
          align-items: center;
          justify-content: center;
          color: #58180d;
          font-size: 0.9rem;
          padding: 2px 0;
          min-height: 1.5em;
      }
  
       .statblock .stats .header-label {
          font-family: ScalaSansCaps, sans-serif;
          font-size: 11px;
          color: #69665f;
          text-transform: uppercase;
          font-weight: bold;
       }
  
        .statblock .stats .header-label.mod { grid-area: mod; }
        .statblock .stats .header-label.save { grid-area: save; }
  
        .statblock .stats .stat {
          font-weight: bold;
          font-family: ScalaSansCaps, sans-serif;
          text-transform: uppercase; /* Match 2024 block */
          font-size: 0.85rem;
          text-align: left;
          padding-left: 5px;
          justify-content: flex-start;
        }
  
        /* Stat Table Colors */
        .statblock .stats .stat.one, .statblock .stats .score.one { background-color: #dcd2ba; }
        .statblock .stats .stat.two, .statblock .stats .score.two { background-color: #d0d5b5; }
        .statblock .stats .mod.one, .statblock .stats .save.one { background-color: #d7c9c2; }
        .statblock .stats .mod.two, .statblock .stats .save.two { background-color: #d4c8db; }
  
        /* Stat Table Grid Areas */
        .statblock .stats .stat.one { grid-area: stat-1; }
        .statblock .stats .score.one { grid-area: score-1; }
        .statblock .stats .mod.one { grid-area: mod-1; }
        .statblock .stats .save.one { grid-area: save-1; }
        .statblock .stats .stat.two { grid-area: stat-2; }
        .statblock .stats .score.two { grid-area: score-2; }
        .statblock .stats .mod.two { grid-area: mod-2; }
        .statblock .stats .save.two { grid-area: save-2; }
  
      /* Section Headers */
      .statblock h3.section {
          font-size: 1.3rem;
          font-family: ScalaSansCaps, sans-serif; /* Fallback */
          font-weight: 400;
          letter-spacing: normal;
          color: #58180d;
          border-bottom: 1px solid #58180d;
          margin-bottom: 4px;
          margin-top: 0.8rem;
          line-height: 1.5rem;
          break-after: avoid;
       }
       .statblock h3.first.section {
          margin-top: 0.5em; /* Less top margin for the first section */
       }
  
       /* Section Content Blocks */
       .statblock .traits,
       .statblock .attacks,
       .statblock .actions,
       .statblock .bonus-actions,
       .statblock .reactions,
       .statblock .legendary-actions,
       .statblock .mythic-actions,
       .statblock .lair-actions,
       .statblock .regional-effects,
       .statblock .innate-spellcasting,
       .statblock .spellcasting,
       .statblock .multiattack {
           break-inside: avoid; /* Prevent breaking inside these blocks */
           margin-bottom: 0.8rem;
       }
  
      /* Individual Items within Sections */
       .statblock .trait,
       .statblock .attack,
       .statblock .action,
       .statblock .legendary-actions .preamble,
       .statblock .mythic-actions .preamble,
       .statblock .lair-actions .preamble,
       .statblock .regional-effects .preamble,
       .statblock .multiattack {
           line-height: 1.15rem;
           margin-bottom: 0.8rem;
       }
  
       /* Bold/Italic Names within items */
       .statblock .trait b i,
       .statblock .attack b i, /* Attack name is usually within b/i tags from template */
       .statblock .action b i,
       .statblock .reaction b i,
       .statblock .legendary b i,
       .statblock .mythic b i {
           font-family: ff-scala-sans-pro, ScalySansBoldItalic, sans-serif;
           font-weight: 700;
           font-style: italic;
       }
  
       /* Italicized parts of attacks */
       .statblock .attack i {
          font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif;
          font-style: italic;
       }
  
       /* Spellcasting Lists */
       .statblock .spell-list {
          margin-top: 0.4rem;
          margin-left: 10px;
       }
  
      .statblock .spell-list .spell-row {
          margin-left: 20px;
          text-indent: -20px;
          margin-bottom: 0.2rem;
      }
  
      .statblock .spell-list .spell-label {
          font-weight: bold;
      }
  
      .statblock .spell-list .spell-list-entries {
          font-family: ff-scala-sans-pro, ScalySansItalic, sans-serif;
          font-style: italic;
      }
  
      /* Legendary/Mythic Actions */
       .statblock .legendary.action, /* Legacy class */
       .statblock .action.legendary { /* More specific class */
          margin-bottom: 0.4rem;
          margin-left: 20px;
          text-indent: -20px;
          line-height: 1.15rem;
       }
  
       /* Lair/Regional Effect Lists */
       .statblock .lair-actions ul,
       .statblock .regional-effects ul {
          list-style: disc;
          margin-left: 30px; /* Indent list */
          padding-left: 0;
       }
       .statblock .lair-actions li,
       .statblock .regional-effects li {
           margin-bottom: 0.5rem;
           line-height: 1.15rem;
       }
  
  </style>