<script setup>
import { ref, computed, watch } from 'vue';

// --- Props ---
const props = defineProps({
  monsterStatBlock: {
    type: Object,
    required: true
  }
});

// --- Reaktive Daten ---
const imageUrl = ref(null);
const imageError = ref(false);
const isLoading = ref(true);

// --- Computed Properties ---
// Konstruiert die URL zum Bild auf dem Backend-Server
const backendBaseUrl = 'http://localhost:8080'; // Passe an, falls nötig!

const generateImageUrl = (monsterId) => {
  if (!monsterId) return null;
  // Annahme: Monster-ID entspricht Dateiname (ohne Endung)
  // und Backend serviert Dateien aus einem bestimmten Pfad
  // Wichtig: '.png' oder '.jpg' muss zum tatsächlichen Format passen!
  // Du musst im Backend sicherstellen, dass dieser Pfad erreichbar ist.
  return `${backendBaseUrl}/data/statblocks/${encodeURIComponent(monsterId)}.png`; // oder .jpg
};

// --- Methoden ---
function handleImageLoad() {
  isLoading.value = false;
  imageError.value = false;
  console.log("Image loaded:", imageUrl.value);
}

function handleImageError() {
  isLoading.value = false;
  imageError.value = true;
  console.error("Failed to load image:", imageUrl.value);
}

// --- Watcher ---
// Beobachtet Änderungen am 'monsterStatBlock' Prop.
// Wenn sich das Monster ändert, aktualisiere die Bild-URL.
watch(() => props.monsterStatBlock, (newStatBlock) => {
  imageError.value = false;
  isLoading.value = true;
  // Verwende eine eindeutige ID oder den Namen aus dem StatBlock
  const monsterId = newStatBlock?.id || newStatBlock?.name?.toLowerCase().replace(/\s+/g, '_'); // Beispielhafte ID-Generierung
  imageUrl.value = generateImageUrl(monsterId);
}, { immediate: true }); // 'immediate: true' führt den Watcher sofort beim Initialisieren aus

</script>

<template>
  <v-card variant="outlined">
    <v-card-title>{{ monsterStatBlock.name || 'Unknown Monster' }}</v-card-title>
    <v-card-text>
      <!-- Zeige Ladeanzeige -->
      <div v-if="isLoading" class="text-center pa-4">
        <v-progress-circular indeterminate color="primary"></v-progress-circular>
        <p>Loading stat block image...</p>
      </div>

      <!-- Zeige Fehlermeldung, wenn Bild nicht geladen werden konnte -->
      <v-alert v-else-if="imageError" type="error" density="compact">
        Could not load stat block image for "{{ monsterStatBlock.name }}".<br>
        Tried URL: {{ imageUrl }} <br>
        Ensure the image exists on the backend at the correct path and the backend is serving static files.
      </v-alert>

      <!-- v-img: Zeigt das Bild an -->
      <v-img
        v-else-if="imageUrl"
        :src="imageUrl"
        :alt="`Stat block for ${monsterStatBlock.name}`"
        contain        
        max-height="600" 
        @load="handleImageLoad"  
        @error="handleImageError" 
      >
         <!-- Optional: Platzhalter, während das Bild lädt -->
         <template v-slot:placeholder>
           <div class="d-flex align-center justify-center fill-height">
             <v-progress-circular indeterminate color="grey-lighten-4"></v-progress-circular>
           </div>
         </template>
      </v-img>

       <div v-else>
         <p>No image URL could be generated for this monster.</p>
       </div>

    </v-card-text>
  </v-card>
</template>

<style scoped>
.v-card-title {
  border-bottom: 1px solid rgba(255, 255, 255, 0.12); /* Leichte Trennlinie */
}
</style>