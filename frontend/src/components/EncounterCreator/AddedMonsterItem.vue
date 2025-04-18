<script setup>
const props = defineProps({
  monster: { // Enthält { monsterId, name, cr, count }
    type: Object,
    required: true
  }
});

const emit = defineEmits(['update-count', 'remove-monster']);

function increaseCount() {
  emit('update-count', { monsterId: props.monster.monsterId, newCount: props.monster.count + 1 });
}

function decreaseCount() {
  const newCount = props.monster.count - 1;
  if (newCount > 0) {
      emit('update-count', { monsterId: props.monster.monsterId, newCount: newCount });
  } else {
      // Bei 0 oder weniger -> entfernen
      emit('remove-monster', props.monster.monsterId);
      // Alternative: emit('update-count', { monsterId: props.monster.monsterId, newCount: 0 });
      // und die Remove-Logik im Parent lassen. Hier ist das direkte Remove-Event klarer.
  }
}
</script>

<template>
  <v-list-item density="compact">
    <v-list-item-title>{{ monster.name }}</v-list-item-title>
    <v-list-item-subtitle>CR: {{ monster.cr ?? 'N/A' }}</v-list-item-subtitle>

    <template v-slot:append>
      <div class="d-flex align-center">
          <v-btn
             icon="mdi-minus"
             variant="text"
             color="secondary"
             size="x-small"
             @click.stop="decreaseCount"
             aria-label="Decrease count"
             class="mr-1"
          ></v-btn>
          <span class="mx-1 font-weight-bold">{{ monster.count }}</span>
          <v-btn
             icon="mdi-plus"
             variant="text"
             color="secondary"
             size="x-small"
             @click.stop="increaseCount"
             aria-label="Increase count"
             class="ml-1"
          ></v-btn>
      </div>
    </template>
  </v-list-item>
</template>

<style scoped>
/* Stile */
</style>