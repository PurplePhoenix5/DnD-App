<!-- frontend/src/components/MonsterCreator/Inventory/Inventory.vue -->
<script setup>
import { computed } from 'vue';

const props = defineProps({
  modelValue: { type: String, default: '' }, // Erhält den inventory String
  isEnabled: { type: Boolean, default: false }
});
const emit = defineEmits(['update:field']);

const inventoryText = computed({
    get: () => props.modelValue,
    set: (value) => emit('update:field', { key: 'inventory', value: value || '' })
});

</script>

<template>
    <div>
        <v-textarea
            label="Inventory, Gear, Armor, Loot... (Markdown: **Bold** and _Italic_)"
            v-model="inventoryText"
            variant="outlined"
            rows="4" 
            auto-grow 
            clearable
            :disabled="!isEnabled"
            density="compact"
            hide-details="auto"
        />
    </div>
</template>

<style scoped>
/* Optional: Styles */
</style>