<script setup>
// Weise das Ergebnis von defineProps der Konstante 'props' zu
const props = defineProps({
  monster: {
    type: Object,
    required: true
  }
});

const emit = defineEmits(['add-monster']);

function addMonster() {
  // Greife jetzt über die definierte 'props'-Konstante zu
  console.log('MonsterListItem: addMonster called, emitting event with:', JSON.parse(JSON.stringify(props.monster)));
  emit('add-monster', props.monster);
}
</script>

<template>
  <v-list-item>
    <!-- Das Template kann weiterhin direkt 'monster' verwenden, da defineProps -->
    <!-- die Props auch direkt für das Template verfügbar macht -->
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