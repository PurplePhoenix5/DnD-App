// src/utils/mathRendering.js
// Based on the provided mathRendering.ts

/**
 * Calculates the average HP based on HD, type, and modifier.
 * @param {object} HP - The HP object { HD: number, type: number, modifier: number }
 * @returns {number} Average HP
 */
export function avgHP(HP) {
    if (!HP) return 1;
    const hd = HP.HD ?? 1;
    const type = HP.type ?? 8;
    const modifier = HP.modifier ?? 0;
    // avg = HD * (avg die roll) + modifier
    return Math.max(1, Math.floor(hd * ((type + 1) / 2) + modifier));
  }
  
  /**
   * Calculates the average result of rolling a number of dice.
   * @param {number} count - Number of dice
   * @param {number} dice - Type of dice (e.g., 6 for d6)
   * @returns {number} Average roll result
   */
  export function avgRoll(count, dice) {
    const numCount = Number(count);
    const numDice = Number(dice);
    if (isNaN(numCount) || isNaN(numDice) || numCount <= 0) return 0;
    if (numDice === 1) return numCount; // Average of d1 is 1
    // avg = count * (avg die roll)
    return Math.floor(numCount * ((numDice + 1) / 2));
  }
  
  /**
   * Calculates the modifier for an ability score.
   * @param {number} score - The ability score
   * @returns {number} The modifier
   */
  export function statModifier(score) {
    return Math.floor(((score ?? 10) - 10) / 2); // Default score 10 if null/undefined
  }
  
  /**
   * Calculates the save modifier for a specific stat, considering proficiency and overrides.
   * @param {object} monster - The monster object
   * @param {string} stat - The stat key (e.g., 'STR', 'DEX')
   * @returns {number} The save modifier
   */
  export function saveModifierForStat(monster, stat) {
    if (!monster?.saves?.[stat]) return statModifier(monster?.stats?.[stat]); // Fallback to just stat mod
    if (monster.saves[stat].override) {
      return monster.saves[stat].overrideValue ?? 0;
    } else {
      const score = monster.stats?.[stat] ?? 10;
      const profBonus = monster.saves[stat].proficient ? (monster.proficiency ?? 2) : 0;
      return statModifier(score) + profBonus;
    }
  }
  
  /**
   * Calculates the save DC for an action, considering stat and overrides.
   * @param {object} monster - The monster object
   * @param {string} stat - The relevant stat key for the DC
   * @param {object} override - The save override object from the action
   * @returns {number} The save DC
   */
  export function saveForAction(monster, stat, override) {
     if (override?.override) {
      return override.overrideValue ?? 10; // Default 10 if overrideValue missing
    } else {
      if (!monster || !stat || stat === 'none') return 8 + (monster?.proficiency ?? 2); // Default DC if no stat
      const score = monster.stats?.[stat] ?? 10;
      return 8 + (monster.proficiency ?? 2) + statModifier(score);
    }
  }
  
  /**
   * Calculates the bonus for a specific skill, considering proficiency, expertise, and overrides.
   * @param {object} monster - The monster object
   * @param {object} skill - The skill object *from the monster's skill list*
   * @returns {number} The skill bonus
   */
  export function bonusForSkill(monster, skill) {
    if (!monster || !skill?.skill) return 0; // Need skill definition
    if (skill.override) {
      return skill.overrideValue ?? 0;
    } else {
      const profBonus = monster.proficiency ?? 2;
      const profMultiplier = skill.expertise ? 2 : skill.proficient ? 1 : 0;
      const score = monster.stats?.[skill.skill.stat] ?? 10;
      return statModifier(score) + (profBonus * profMultiplier);
    }
  }
  
  /**
   * Calculates the attack bonus for a specific attack, considering proficiency and overrides.
   * @param {object} monster - The monster object
   * @param {object} attack - The attack object
   * @returns {number} The attack bonus
   */
  export function bonusForAttack(monster, attack) {
    if (!monster || !attack?.modifier) return 0;
    if (attack.modifier.override) {
      return attack.modifier.overrideValue ?? 0;
    } else {
      const profBonus = attack.modifier.proficient ? (monster.proficiency ?? 2) : 0;
      const score = monster.stats?.[attack.modifier.stat] ?? 10;
      return statModifier(score) + profBonus;
    }
  }
  
  /**
   * Calculates the damage bonus for an attack's primary damage, considering overrides.
   * @param {object} monster - The monster object
   * @param {object} attack - The attack object
   * @returns {number} The damage bonus
   */
  export function bonusForAttackDamage(monster, attack) {
     if (!monster || !attack?.damage?.modifier) return 0; // Check if modifier object exists
     if (attack.damage.modifier.override) {
      return attack.damage.modifier.overrideValue ?? 0;
    } else {
      // Use the specific stat from damage.modifier, fallback to attack.modifier.stat, then STR
      const statKey = attack.damage.modifier.stat || attack.modifier?.stat || 'STR';
      const score = monster.stats?.[statKey] ?? 10;
      return statModifier(score);
    }
  }
  
  /**
   * Calculates the damage bonus for an attack's alternate/conditional damage, considering overrides.
   * @param {object} monster - The monster object
   * @param {object} attack - The attack object
   * @returns {number} The conditional damage bonus
   */
  export function bonusForConditionalDamage(monster, attack) {
     // Check if alternate damage exists and has a modifier object
    if (!monster || !attack?.alternateDamage?.modifier) return 0;
    if (attack.alternateDamage.modifier.override) {
      return attack.alternateDamage.modifier.overrideValue ?? 0;
    } else {
       // Use the specific stat from alternateDamage.modifier, fallback to attack.modifier.stat, then STR
      const statKey = attack.alternateDamage.modifier.stat || attack.modifier?.stat || 'STR';
      const score = monster.stats?.[statKey] ?? 10;
      return statModifier(score);
    }
  }
  
  /**
   * Renders a number as a bonus string (e.g., +3, -1, +0).
   * @param {number} number - The bonus value
   * @param {boolean} [spaces=false] - Whether to add spaces around the sign
   * @returns {string} The formatted bonus string
   */
  export function renderBonus(number, spaces = false) {
    const num = Number(number); // Ensure it's a number
    if (isNaN(num)) return '+0'; // Fallback for invalid input
    const sign = num >= 0 ? '+' : ''; // Negative numbers have their own sign
    const spaceBefore = spaces ? ' ' : '';
    const spaceAfter = spaces ? ' ' : '';
    return `${spaceBefore}${sign}${spaceAfter}${num}`;
  }