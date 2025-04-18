// src/utils/dndData/skills.js
export const SKILLS = {
    ACROBATICS: { stat: 'DEX', label: 'Acrobatics' },
    ANIMAL_HANDLING: { stat: 'WIS', label: 'Animal Handling' },
    ARCANA: { stat: 'INT', label: 'Arcana' },
    ATHLETICS: { stat: 'STR', label: 'Athletics' },
    DECEPTION: { stat: 'CHA', label: 'Deception' },
    HISTORY: { stat: 'INT', label: 'History' },
    INITIATIVE: { stat: 'DEX', label: 'Initiative' }, // Needed for logic
    INSIGHT: { stat: 'WIS', label: 'Insight' },
    INTIMIDATION: { stat: 'CHA', label: 'Intimidation' },
    INVESTIGATION: { stat: 'INT', label: 'Investigation' },
    MEDICINE: { stat: 'WIS', label: 'Medicine' },
    NATURE: { stat: 'INT', label: 'Nature' },
    PERCEPTION: { stat: 'WIS', label: 'Perception' },
    PERFORMANCE: { stat: 'CHA', label: 'Performance' },
    PERSUASION: { stat: 'CHA', label: 'Persuasion' },
    RELIGION: { stat: 'INT', label: 'Religion' },
    SLIGHT_OF_HAND: { stat: 'DEX', label: 'Sleight of Hand' },
    STEALTH: { stat: 'DEX', label: 'Stealth' },
    SURVIVAL: { stat: 'WIS', label: 'Survival' },
};

export const SKILL_KEYS = Object.keys(SKILLS);