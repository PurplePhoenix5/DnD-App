<script setup>
defineProps({
  monster: {
    type: Object,
    required: true
  }
});

const emit = defineEmits(['add-monster']);

function addMonster() {
  emit('add-monster', props.monster); // Sende das *ganze* Monster-Objekt (oder nur die ID)
}
</script>

<template>
  <v-list-item>
    <v-list-item-title>{{ monster.name }}</v-list-item-title>
    <v-list-item-subtitle>
       CR: {{ monster.cr ?? 'N/A' }} | Size: {{ monster.size ?? 'N/A' }} | Type: {{ monster.type ?? 'N/A' }}
    </v-list-item-subtitle>

    <template v-slot:append>
      <v-btn
          icon="mdi-plus"
          variant="text"
          color="primary"
          size="small"
          @click.stop="addMonster" 
          aria-label="Add monster"
      ></v-btn>
    </template>
  </v-list-item>
</template>

<style scoped>
/* Optional: Stile */
</style>