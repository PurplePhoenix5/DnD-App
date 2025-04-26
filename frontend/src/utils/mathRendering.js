// frontend/src/utils/mathRendering.js

/**
 * Calculates the modifier for an ability score.
 * @param {number | null | undefined} score - The ability score
 * @returns {number} The modifier
 */
export function statModifier(score) {
  // Sichere Behandlung, falls score null/undefined ist
  const numericScore = Number(score);
  if (isNaN(numericScore)) return 0; // Oder einen anderen Default-Wert
  return Math.floor((numericScore - 10) / 2);
}

/**
 * Calculates the initiative bonus.
 * @param {object} monster - The monster object (needs stats.DEX, proficiency)
 * @param {object} initiativeConfig - The initiative config { initProficiency: boolean, initExpertise: boolean }
 * @returns {number} The calculated initiative bonus
 */
export function calculateInitiativeBonus(monster, initiativeConfig) {
    if (!monster || !initiativeConfig) return 0;
    const dexMod = statModifier(monster.stats?.DEX); // Nutze die vorhandene Funktion
    const profBonus = monster.PB ?? 2; // Greife auf PB im basics-Objekt zu

    let initiativeBonus = dexMod;
    if (initiativeConfig.initExpertise) {
        initiativeBonus += profBonus * 2;
    } else if (initiativeConfig.initProficiency) {
        initiativeBonus += profBonus;
    }
    return initiativeBonus;
}

/**
 * Renders a number as a bonus string (e.g., +3, -1, +0).
 * @param {number | null | undefined} number - The bonus value
 * @param {boolean} [spaces=false] - Whether to add spaces around the sign
 * @returns {string} The formatted bonus string
 */
export function renderBonus(number, spaces = false) {
    const num = Number(number); // Ensure it's a number
    if (isNaN(num)) return '+0'; // Fallback for invalid input
    const sign = num >= 0 ? '+' : ''; // Negative numbers have their own sign
    const spaceBefore = spaces ? ' ' : '';
    const spaceAfter = spaces ? ' ' : '';
    return `${spaceBefore}${sign}${num}`; // Verwende num, nicht number, da es jetzt garantiert eine Zahl ist
  }

  /**
 * Calculates the average result of rolling a number of dice.
 * @param {number | null | undefined} count - Number of dice
 * @param {number | null | undefined} dice - Type of dice (e.g., 6 for d6)
 * @returns {number} Average roll result
 */
export function avgRoll(count, dice) {
    const numCount = Number(count);
    const numDice = Number(dice);
    if (isNaN(numCount) || isNaN(numDice) || numCount <= 0 || numDice <= 0) return 0;
    if (numDice === 1) return numCount;
    return Math.floor(numCount * ((numDice + 1) / 2));
  }
  
  /**
   * Calculates the average HP based on HD, type (or override), and modifier.
   * @param {object} basicsData - The basics object from the monster { HP: { HDAmount, defaultDie, overrideDie, HPmodifier } }
   * @returns {number} Average HP
   */
  export function avgHP(basicsData) {
      if (!basicsData?.HP) return 1;
      const HP = basicsData.HP;
      const hd = HP.HDAmount ?? 1;
      const type = HP.overrideDie ?? HP.defaultDie ?? 8; // Use override if available
      const modifier = HP.HPmodifier ?? 0;
      return Math.max(1, Math.floor(hd * ((type + 1) / 2)) + modifier); // Use avgRoll logic internally
  }
  
  /**
 * Calculates the save modifier for a specific stat, considering proficiency and overrides.
 * Uses the new monster template structure.
 * @param {object} monster - The *full* monster object (needs basics.stats, basics.PB, saves)
 * @param {string} stat - The stat key (e.g., 'STR', 'DEX')
 * @returns {number} The save modifier
 */
export function saveModifierForStat(monster, stat) {
  const statKey = stat?.toUpperCase();
  if (!monster || !statKey || !monster.basics || !monster.saves) return 0;

  const saveInfo = monster.saves[statKey];
  const basics = monster.basics;

  const score = basics.stats?.[statKey] ?? 10;
  const profBonus = saveInfo?.proficient ? (basics.PB ?? 2) : 0;
  return statModifier(score) + profBonus;
}
  
  /**
   * Calculates the save DC for an action based on a stat.
   * Uses the new monster template structure.
   * @param {object} monster - The *full* monster object (needs basics.stats, basics.PB)
   * @param {string} stat - The relevant stat key for the DC (e.g., 'INT', 'WIS')
   * @returns {number} The save DC
   */
  export function calculateSaveDC(monster, stat) {
      const statKey = stat?.toUpperCase();
      if (!monster || !monster.basics || !statKey || statKey === 'none') {
          return 8 + (monster.basics?.PB ?? 2); // Default DC if no stat
      }
      const score = monster.basics.stats?.[statKey] ?? 10;
      return 8 + (monster.basics.PB ?? 2) + statModifier(score);
  }
  
  /**
   * Calculates the bonus for a specific skill, considering proficiency, expertise, and overrides.
   * Uses the new monster template structure.
   * @param {object} monster - The *full* monster object (needs basics.stats, basics.PB, skills)
   * @param {object} skillInfo - The skill object from the monster's skills array { skill: string, proficient: boolean, expertise: boolean, overrideValue: number | null }
   * @param {object} skillsData - The loaded skills data mapping skill names to stats (optional, but recommended for stat lookup)
   * @returns {number} The skill bonus
   */
  export function bonusForSkill(monster, skillInfo, skillsData = {}) {
      if (!monster || !monster.basics || !skillInfo?.skill) return 0;
      if (skillInfo.overrideValue !== null && skillInfo.overrideValue !== undefined) {
        return skillInfo.overrideValue;
      }
  
      // Finde den zugehörigen Stat (benötigt geladene skillsData)
      const skillDefinition = skillsData[skillInfo.skill];
      const statKey = skillDefinition?.stat || 'INT'; // Fallback auf INT oder was sinnvoll ist
  
      const profBonus = monster.basics.PB ?? 2;
      const profMultiplier = skillInfo.expertise ? 2 : skillInfo.proficient ? 1 : 0;
      const score = monster.basics.stats?.[statKey] ?? 10;
  
      return statModifier(score) + (profBonus * profMultiplier);
  }
  
  /**
   * Calculates the attack bonus for an Attack Roll action.
   * Uses the new monster template structure.
   * @param {object} monster - The *full* monster object (needs basics.stats, basics.PB)
   * @param {object} attackAction - The attackRoll object from monster.actions or monster.bonusAction
   * @returns {number} The attack bonus (to hit modifier)
   */
  export function bonusForAttack(monster, attackAction) {
      // Im neuen Template ist attackMod ein fester Wert, keine Berechnung mehr nötig?
      // Falls doch berechnet werden soll, bräuchten wir Logik ähnlich wie saveModifierForStat
      // unter Annahme, dass attackAction eine 'stat'- und 'proficient'-Property hätte.
      // Hier gehen wir davon aus, der Wert ist im Template vorgegeben:
      return attackAction?.attackMod ?? (monster?.basics?.PB ?? 2); // Fallback auf PB? Oder 0?
  }
  
  /**
   * Calculates the primary damage bonus for an Attack Roll action.
   * Uses the new monster template structure (nimmt den ersten damage Eintrag).
   * @param {object} monster - The *full* monster object (needs basics.stats)
   * @param {object} attackAction - The attackRoll object
   * @returns {number} The damage bonus
   */
  export function bonusForAttackDamage(monster, attackAction) {
       // Im neuen Template ist der Modifier direkt beim Damage-Eintrag.
       // Nehme den Modifier des *ersten* Damage-Eintrags.
       const firstDamage = attackAction?.damage?.[0];
       if (!firstDamage) return 0;
       return firstDamage.modifier ?? 0; // Gehe davon aus, Modifier ist ein fester Wert
  
       // Falls der Modifier *doch* aus Stats berechnet werden soll:
       // const statKey = firstDamage.stat || 'STR'; // Annahme: 'stat' Property existiert im damage object
       // const score = monster?.basics?.stats?.[statKey] ?? 10;
       // return statModifier(score);
  }
  
  // bonusForConditionalDamage existiert im neuen Template nicht direkt.
  // Der Schaden wird pro Eintrag im 'damage'-Array definiert.
  // Falls benötigt, müsste man einen spezifischen Eintrag im damage-Array übergeben.
