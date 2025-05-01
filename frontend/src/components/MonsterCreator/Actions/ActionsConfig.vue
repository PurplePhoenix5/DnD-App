<!-- frontend/src/components/MonsterCreator/Actions/ActionsConfig.vue -->
<script setup>
import ActionConfigBase from './ActionConfigBase.vue';

const props = defineProps({
    modelValue: { type: Object, required: true }, // Erhält das flache Array monsterData.actions
    basicsData: { type: Object, required: true },
    isEnabled: { type: Boolean, default: false }
});

const emit = defineEmits(['update:field']); // Leitet das Event weiter

function handleBaseUpdate(payload) { // payload ist { key: 'actions', value: [...] }
    emit('update:field', payload); // Leite direkt weiter (value ist schon das Array)
}

</script>
<template>
    <ActionConfigBase
        :modelValue="modelValue"
        actionGroupType="actions"
        :basicsData="basicsData"
        :isEnabled="isEnabled"
        @update:field="handleBaseUpdate" 
    />
</template>