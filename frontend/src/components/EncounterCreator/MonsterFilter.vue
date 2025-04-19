<script setup>
import { ref, watch } from 'vue';

const emit = defineEmits(['filter-changed']);

// Lokale Refs für die Filterwerte
const nameSearch = ref('');
const crRange = ref([0, 35]); // Initialer CR Bereich

// Beobachte Änderungen und sende Event (mit Debounce, um nicht bei jedem Tastendruck zu senden)
let debounceTimer = null;
watch([nameSearch, crRange], () => {
  clearTimeout(debounceTimer);
  debounceTimer = setTimeout(() => {
    emit('filter-changed', {
      name: nameSearch.value,
      crRange: crRange.value
    });
  }, 300); // Warte 300ms nach der letzten Änderung
}, { deep: true }); // deep: true ist wichtig für das Array crRange

</script>

<template>
  <v-card variant="outlined">
    <v-card-title>Filter Monsters</v-card-title>
    <v-card-text>
      <v-text-field
        v-model="nameSearch"
        label="Search by Name"
        prepend-inner-icon="mdi-magnify"
        clearable
        density="compact"
        variant="outlined"
        hide-details
        class="mb-4"
      />
      <v-range-slider
        v-model="crRange"
        label="CR Range"
        :min="0"
        :max="35" 
        :step="0.125" 
        thumb-label="always" 
        density="compact"
        hide-details
      >
         <!-- Zeigt die aktuellen Werte im Label an -->
         <template v-slot:thumb-label="{ modelValue }">
            {{ modelValue % 1 === 0 ? modelValue : modelValue.toFixed(3).replace(/0+$/,'') }} <!-- Zeigt Brüche besser an -->
         </template>
      </v-range-slider>
    </v-card-text>
  </v-card>
</template>

<style scoped>
/* Optional: Stile */
</style>