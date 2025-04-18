<script setup>
import { ref, onMounted } from 'vue';

// Reaktive Variablen für die Daten vom Backend
const status = ref('Lade...'); // Startwert, während wir warten
const message = ref('');
const error = ref(null); // Um eventuelle Fehler zu speichern

// Funktion zum Abrufen der Daten vom Backend
async function fetchStatus() {
  error.value = null; // Fehler zurücksetzen
  try {
    // WICHTIG: Die URL muss die Adresse deines C++ Backends sein.
    // Wenn Frontend (npm run dev) auf Port 5173 läuft und Backend auf 8080,
    // dann ist die URL http://localhost:8080/api/status
    // Passe den Port ggf. an!
    const response = await fetch('http://localhost:8080/api/status');

    if (!response.ok) {
      // Fehler beim Abrufen (z.B. 404 Not Found, 500 Server Error)
      throw new Error(`HTTP Fehler! Status: ${response.status}`);
    }

    // Wandle die Antwort in JSON um
    const data = await response.json(); 

    // Aktualisiere die reaktiven Variablen mit den Daten aus der Antwort
    status.value = data.status;
    message.value = data.message;

  } catch (err) {
    // Fehler beim fetch oder JSON-Parsing
    console.error('Fehler beim Abrufen des Status:', err);
    error.value = 'Fehler beim Laden der Daten vom Backend. Läuft das Backend auf Port 8080? Details siehe Konsole.';
    status.value = 'Fehler';
    message.value = '';
  }
}

// 'onMounted' ist ein Vue Lifecycle Hook.
// Die Funktion darin wird ausgeführt, sobald die Komponente
// zum ersten Mal in die Seite eingefügt (gemountet) wurde.
// Perfekt, um initiale Daten zu laden.
onMounted(() => {
  fetchStatus(); // Rufe die Funktion auf, wenn die Komponente bereit ist.
});
</script>

<template>
  <v-card variant="tonal">
    <v-card-title>Backend Status</v-card-title>
    <v-card-text>
      <div v-if="error">
        <v-alert type="error" dense>
          {{ error }}
        </v-alert>
      </div>
      <div v-else>
        <p><strong>Status:</strong> {{ status }}</p>
        <p><strong>Nachricht:</strong> {{ message }}</p>
      </div>
      <v-btn @click="fetchStatus" :disabled="status === 'Lade...'" small class="mt-4">
        Status erneut laden
      </v-btn>
    </v-card-text>
  </v-card>
</template>

<style scoped>
/* Stile nur für diese Komponente */
p {
  margin-bottom: 8px;
}
</style>