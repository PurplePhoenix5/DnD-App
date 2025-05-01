<!-- frontend/src/components/MonsterCreator/Actions/ActionsConfig.vue -->
<script setup>
import ActionConfigBase from './ActionConfigBase.vue'; // Passe Pfad an falls nötig

const props = defineProps({
    modelValue: { type: Array, required: true }, // Erhält monsterData.actions (als Array)
    basicsData: { type: Object, required: true }, // Für Save DC Berechnung
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']); // Emittiert { key: 'actions', value: [...] }

</script>

<template>
    <ActionConfigBase
        :modelValue="modelValue"
        actionGroupType="actions"
        :basicsData="basicsData"
        :isEnabled="isEnabled"
        @update:field="emit('update:field', $event)"
    />
</template>

<style scoped>
/* Hier können spezifische Styles für ActionsConfig stehen, falls nötig */
/* Oft ist die Basis-Komponente ausreichend */
</style>