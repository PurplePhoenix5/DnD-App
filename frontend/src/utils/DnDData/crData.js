// src/utils/dndData/crData.js
const rawCR = [
    // ... (Daten aus CR.ts kopieren) ...
    { cr: '0', proficiency: 2, numeric: 0, xp: 0, },
    { cr: '1/8', proficiency: 2, numeric: 0.125, xp: 25, },
    { cr: '1/4', proficiency: 2, numeric: 0.25, xp: 50, },
    { cr: '1/2', proficiency: 2, numeric: 0.5, xp: 100, },
    { cr: '1', proficiency: 2, numeric: 1, xp: 200, },
    { cr: '2', proficiency: 2, numeric: 2, xp: 450, },
    { cr: '3', proficiency: 2, numeric: 3, xp: 700, },
    { cr: '4', proficiency: 2, numeric: 4, xp: 1100, },
    { cr: '5', proficiency: 3, numeric: 5, xp: 1800, },
    { cr: '6', proficiency: 3, numeric: 6, xp: 2300, },
    { cr: '7', proficiency: 3, numeric: 7, xp: 2900, },
    { cr: '8', proficiency: 3, numeric: 8, xp: 3900, },
    { cr: '9', proficiency: 4, numeric: 9, xp: 5000, },
    { cr: '10', proficiency: 4, numeric: 10, xp: 5900, },
    { cr: '11', proficiency: 4, numeric: 11, xp: 7200, },
    { cr: '12', proficiency: 4, numeric: 12, xp: 8400, },
    { cr: '13', proficiency: 5, numeric: 13, xp: 10000, },
    { cr: '14', proficiency: 5, numeric: 14, xp: 11500, },
    { cr: '15', proficiency: 5, numeric: 15, xp: 13000, },
    { cr: '16', proficiency: 5, numeric: 16, xp: 15000, },
    { cr: '17', proficiency: 6, numeric: 17, xp: 18000, },
    { cr: '18', proficiency: 6, numeric: 18, xp: 20000, },
    { cr: '19', proficiency: 6, numeric: 19, xp: 22000, },
    { cr: '20', proficiency: 6, numeric: 20, xp: 25000, },
    { cr: '21', proficiency: 7, numeric: 21, xp: 33000, },
    { cr: '22', proficiency: 7, numeric: 22, xp: 41000, },
    { cr: '23', proficiency: 7, numeric: 23, xp: 50000, },
    { cr: '24', proficiency: 7, numeric: 24, xp: 62000, },
    { cr: '25', proficiency: 8, numeric: 25, xp: 75000, },
    { cr: '26', proficiency: 8, numeric: 26, xp: 90000, },
    { cr: '27', proficiency: 8, numeric: 27, xp: 105000, },
    { cr: '28', proficiency: 8, numeric: 28, xp: 120000, },
    { cr: '29', proficiency: 9, numeric: 29, xp: 135000, },
    { cr: '30', proficiency: 9, numeric: 30, xp: 155000, },
];

export const CR = rawCR.map((cr, index) => ({ index, ...cr }));

export function getCrByNumber(crValue) {
  // Handle numeric comparison carefully due to potential floating point issues
  const found = CR.find(c => Math.abs(c.numeric - crValue) < 0.001);
  if (found) return found;

  // Fallback for specific fractional CR strings if direct numeric match failed
  if (crValue === 0.125) return CR.find(c => c.cr === '1/8');
  if (crValue === 0.25) return CR.find(c => c.cr === '1/4');
  if (crValue === 0.5) return CR.find(c => c.cr === '1/2');

  // Default to CR 0 if not found otherwise
  return CR.find(c => c.numeric === 0);
}

export function getCrByString(crString) {
  return CR.find((cr) => cr.cr === crString);
}