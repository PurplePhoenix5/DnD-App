<!-- frontend/src/components/MonsterCreator/Bonus Actions/BonusActionsConfig.vue -->
<script setup>
import ActionConfigBase from '../Actions/ActionConfigBase.vue'; // Passe Pfad an falls nötig

const props = defineProps({
    modelValue: { type: Array, required: true }, // Erhält monsterData.bonusAction (als Array)
     basicsData: { type: Object, required: true }, // Für Save DC Berechnung
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']); // Emittiert { key: 'bonusAction', value: [...] }

</script>

<template>
    <ActionConfigBase
        :modelValue="modelValue"
        actionGroupType="bonusAction"
         :basicsData="basicsData"
        :isEnabled="isEnabled"
        @update:field="emit('update:field', $event)"
    />
</template>

<style scoped>
/* Hier können spezifische Styles für BonusActionsConfig stehen, falls nötig */
/* Oft ist die Basis-Komponente ausreichend */
</style>