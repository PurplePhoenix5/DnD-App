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
