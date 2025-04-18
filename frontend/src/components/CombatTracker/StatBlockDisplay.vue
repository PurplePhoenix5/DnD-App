<script setup>
import StatBlock from './StatBlock.vue'; // Die Komponente für den einzelnen Stat Block

// --- Props ---
defineProps({
  uniqueMonsterStatBlocks: {
    type: Array,
    required: true,
    default: () => []
  }
});
</script>

<template>
  <div>
    <h3 class="mb-3">Monster Stat Blocks</h3>
    <div v-if="uniqueMonsterStatBlocks.length > 0">
       <!-- Iteriere über die einzigartigen Stat Blocks -->
       <StatBlock
         v-for="statBlock in uniqueMonsterStatBlocks"
         :key="statBlock.name" 
         :monster-stat-block="statBlock"
         class="mb-4" 
       />
       <!-- :key kann hier der Monstername sein, wenn er eindeutig ist, oder eine ID, falls vorhanden -->
       <!-- :monster-stat-block übergibt das volle Objekt an die Kindkomponente -->
    </div>
    <div v-else>
      <p>No monster stat blocks to display. Load an encounter.</p>
    </div>
  </div>
</template>

<style scoped>
/* CSS */
</style>