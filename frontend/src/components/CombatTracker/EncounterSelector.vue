<script setup>
import { ref, watch, onMounted } from 'vue';

// --- Definition der Events ---
// defineEmits deklariert die Events, die diese Komponente aussenden kann.
// 'encounter-selected' wird ausgelöst, wenn ein Benutzer einen Encounter auswählt.
// Der Wert, der mit dem Event gesendet wird, ist die ID des ausgewählten Encounters.
const emit = defineEmits(['encounter-selected']);

// --- Reaktive Daten ---
// ref() erstellt reaktive Variablen. Änderungen an diesen Variablen aktualisieren die UI automatisch.
const availableEncounters = ref([]); // Speichert die Liste der Encounter {id: string, name: string}
const selectedEncounter = ref(null); // Hält die ID des aktuell ausgewählten Encounters
const isLoading = ref(false);        // Zeigt an, ob Daten gerade geladen werden
const error = ref(null);             // Speichert eventuelle Fehlermeldungen

// --- Methoden ---
// Funktion zum Abrufen der Encounter-Liste vom Backend
async function fetchEncounters() {
  isLoading.value = true;
  error.value = null;
  availableEncounters.value = []; // Liste leeren vor dem Neuladen

  try {
    // API-Anfrage an dein Backend
    const response = await fetch('http://localhost:8080/api/encounters'); // Passe Port/URL bei Bedarf an
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    const data = await response.json();
    // Speichere die geladenen Daten in der reaktiven Variable
    availableEncounters.value = data;
  } catch (err) {
    console.error('Error fetching encounters:', err);
    error.value = 'Failed to load encounters. Is the backend running?';
  } finally {
    isLoading.value = false; // Ladeanzeige beenden, egal ob erfolgreich oder nicht
  }
}

// --- Lifecycle Hook ---
// onMounted() ist ein Vue Hook. Die Funktion darin wird ausgeführt,
// *nachdem* die Komponente zum ersten Mal in die Seite eingefügt wurde.
// Ideal, um initiale Daten zu laden.
onMounted(() => {
  fetchEncounters();
});

// --- Watcher ---
// watch() beobachtet Änderungen an einer reaktiven Variable.
// Hier beobachten wir `selectedEncounter`. Wenn sich der Wert ändert
// (weil der Benutzer etwas ausgewählt hat), senden wir das Event nach oben.
watch(selectedEncounter, (newId) => {
  if (newId) {
    // Sende das Event 'encounter-selected' mit der neuen ID als Payload
    emit('encounter-selected', newId);
  }
});

</script>

<template>
  <div>
    <!-- v-autocomplete: Dropdown-Liste mit Suchfunktion -->
    <v-autocomplete
      v-model="selectedEncounter"
      :items="availableEncounters"
      item-title="name"        
      item-value="id"         
      label="Select Encounter"
      :loading="isLoading"     
      :disabled="isLoading"    
      placeholder="Search or select an encounter..."
      clearable               
      variant="outlined"
      density="compact"
      hide-details="auto"     
    >
      <!-- Anpassung, wie die Items in der Liste aussehen (optional) -->
      <template v-slot:item="{ props, item }">
        <v-list-item v-bind="props" :title="item.raw.name"></v-list-item>
      </template>

       <!-- Zeigt "No data available" wenn keine Encounter geladen wurden -->
      <template v-slot:no-data>
        <v-list-item>
          <v-list-item-title>
            No encounters found. Check backend or try refreshing.
          </v-list-item-title>
        </v-list-item>
      </template>
    </v-autocomplete>

    <!-- Zeige eine Fehlermeldung an, falls das Laden fehlschlug -->
    <v-alert v-if="error" type="warning" density="compact" class="mt-2">
      {{ error }}
    </v-alert>
  </div>
</template>

<style scoped>
/* CSS-Regeln hier gelten nur für diese Komponente */
</style>