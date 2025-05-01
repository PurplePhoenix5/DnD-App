<!-- frontend/src/components/MonsterCreator/Bonus Actions/BonusActionsConfig.vue -->
<script setup>
import ActionConfigBase from '../Actions/ActionConfigBase.vue'; // Korrekter Pfad

const props = defineProps({
    modelValue: { type: Object, required: true }, // Erhält das flache Array monsterData.bonusAction
    basicsData: { type: Object, required: true },
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']); // Leitet das Event weiter

function handleBaseUpdate(payload) { // payload ist { key: 'bonusAction', value: [...] }
    emit('update:field', payload); // Leite direkt weiter
}

</script>
<template>
    <ActionConfigBase
        :modelValue="modelValue"
        actionGroupType="bonusAction"
        :basicsData="basicsData"
        :isEnabled="isEnabled"
        @update:field="handleBaseUpdate" 
    />
</template>