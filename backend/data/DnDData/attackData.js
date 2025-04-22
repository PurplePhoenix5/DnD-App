// src/utils/dndData/attackData.js
export const ATTACK_RANGES = { MELEE: 'Melee', RANGED: 'Ranged', BOTH: 'Melee or Ranged' };
export const ATTACK_KINDS = { WEAPON: 'Weapon', SPELL: 'Spell' };
export const ATTACK_RANGE_OPTIONS = Object.entries(ATTACK_RANGES).map(([value, label]) => ({ value, label }));
export const ATTACK_KIND_OPTIONS = Object.entries(ATTACK_KINDS).map(([value, label]) => ({ value, label }));