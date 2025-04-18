<script setup>
import MonsterListItem from './MonsterListItem.vue';

// Empfängt die gefilterte Monsterliste
defineProps({
  monsters: {
    type: Array,
    required: true
  }
});

// Definiert das Event, das nach oben weitergeleitet wird
const emit = defineEmits(['add-monster']);

function handleAddMonster(monster) {
  emit('add-monster', monster);
}
</script>

<template>
  <v-card variant="outlined">
    <v-card-title>Available Monsters</v-card-title>
    <v-list density="compact" lines="two">
       <!-- V-virtual-scroll wäre besser bei SEHR vielen Monstern -->
      <MonsterListItem
        v-for="monster in monsters"
        :key="monster.id"
        :monster="monster"
        @add-monster="handleAddMonster"
      />
      <v-list-item v-if="monsters.length === 0">
         <v-list-item-title class="text-center text-disabled">No monsters match filters.</v-list-item-title>
      </v-list-item>
    </v-list>
  </v-card>
</template>

<style scoped>
.v-list {
  max-height: 60vh; /* Beispiel: Begrenze Höhe und erlaube Scrollen */
  overflow-y: auto;
}
</style>