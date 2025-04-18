<script setup>
import AddedMonsterItem from './AddedMonsterItem.vue';

defineProps({
  addedMonsters: {
    type: Array, // Format: [{ monsterId, name, cr, count }, ...]
    required: true
  }
});

const emit = defineEmits(['update-count', 'remove-monster']);

function handleUpdateCount(payload) {
  emit('update-count', payload);
}
function handleRemoveMonster(monsterId) {
    emit('remove-monster', monsterId); // Einfach weiterleiten
}

</script>

<template>
  <div>
     <h4 class="mb-2">Added Monsters</h4>
     <v-list density="compact" lines="one" v-if="addedMonsters.length > 0">
       <AddedMonsterItem
         v-for="monster in addedMonsters"
         :key="monster.monsterId"
         :monster="monster"
         @update-count="handleUpdateCount"
         @remove-monster="handleRemoveMonster"
       />
     </v-list>
     <p v-else class="text-disabled text-center">No monsters added yet.</p>
  </div>
</template>

<style scoped>
.v-list {
    background: transparent; /* Optional: Kein eigener Hintergrund für die Liste */
}
</style>