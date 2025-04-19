// src/utils/dndData/classes.js
export const SRD_CLASSES = {
    BARBARIAN: 'Barbarian', BARD: 'Bard', CLERIC: 'Cleric', DRUID: 'Druid',
    FIGHTER: 'Fighter', MONK: 'Monk', PALADIN: 'Paladin', RANGER: 'Ranger',
    ROGUE: 'Rogue', SORCERER: 'Sorcerer', WARLOCK: 'Warlock', WIZARD: 'Wizard',
};

// Kopiere SpellSlotsByLevel aus CLASS.ts hierher
export const SpellSlotsByLevel = {
    ARTIFICER: [ /* ... */ [4, 3, 3, 3, 2, 0, 0, 0, 0] ], // Example: Last entry
    HALF: [ /* ... */ [4, 3, 3, 3, 2, 0, 0, 0, 0] ],
    FULL: [ /* ... */ [4, 3, 3, 3, 3, 2, 2, 1, 1] ],
    WARLOCK: [ /* ... */ [0, 0, 0, 0, 4, 0, 0, 0, 0] ],
};

export const ClassCastingStat = {
    BARD: 'CHA', CLERIC: 'WIS', DRUID: 'WIS', PALADIN: 'CHA', RANGER: 'WIS',
    SORCERER: 'CHA', WIZARD: 'INT', WARLOCK: 'CHA',
};

export const ClassSpellSlots = {
    BARD: SpellSlotsByLevel.FULL, CLERIC: SpellSlotsByLevel.FULL, DRUID: SpellSlotsByLevel.FULL,
    PALADIN: SpellSlotsByLevel.HALF, RANGER: SpellSlotsByLevel.HALF, SORCERER: SpellSlotsByLevel.FULL,
    WIZARD: SpellSlotsByLevel.FULL, WARLOCK: SpellSlotsByLevel.WARLOCK,
};

export const SrdCastingClassOptions = Object.entries(SRD_CLASSES)
    .filter(([value]) => value in ClassSpellSlots)
    .map(([value, name]) => ({ value: value, label: name }));