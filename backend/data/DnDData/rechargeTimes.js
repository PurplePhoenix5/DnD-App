// src/utils/dndData/rechargeTimes.js
const RECHARGE_LABELS = {
    REST_SHORT: 'Short Rest',
    REST_LONG: 'Long Rest',
    DAY: 'Day',
    DAWN: 'Dawn',
    ROUND: 'Round',
    AT_WILL: 'At Will',
    LONG_OR_SHORT: 'Long or Short Rest', // Added from template file
    // Add other keys if present in your data (e.g., TURN)
};
export const RECHARGE_TIME_KEYS = Object.keys(RECHARGE_LABELS);
export const RECHARGE_TIME_OPTIONS = RECHARGE_TIME_KEYS.map(k => ({ label: RECHARGE_LABELS[k], value: k }));
export function getRechargeLabel(key) {
    return RECHARGE_LABELS[key] || key; // Fallback to key if not found
}