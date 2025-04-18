<script setup>
import { ref, onMounted, watch } from 'vue';

// Props für die Datenbindung mit v-model (oder direkt über Events)
const props = defineProps({
    name: String,
    description: String,
    partySize: Number,
    partyLevel: Number,
    loading: Boolean, // Für Ladeanzeige beim Laden
    error: String    // Für Fehlermeldungen beim Laden
});

// Events zum Aktualisieren der Werte im Parent
const emit = defineEmits([
    'update:name',
    'update:description',
    'update:partySize',
    'update:partyLevel',
    'load-encounter',
    'clear-encounter' // Event zum Leeren des Formulars
]);

// Lokale Refs für die Eingabefelder (damit v-model funktioniert)
// Wir müssen watch verwenden, um die lokalen Refs zu aktualisieren, wenn sich die Props ändern (beim Laden)
const localName = ref(props.name);
const localDescription = ref(props.description);
const localPartySize = ref(props.partySize);
const localPartyLevel = ref(props.partyLevel);

watch(() => props.name, (newName) => localName.value = newName);
watch(() => props.description, (newDesc) => localDescription.value = newDesc);
watch(() => props.partySize, (newSize) => localPartySize.value = newSize);
watch(() => props.partyLevel, (newLevel) => localPartyLevel.value = newLevel);


// Ref für die Liste der existierenden Encounter und das ausgewählte zum Laden
const existingEncounters = ref([]); // Format: [{ id: '...', name: '...' }]
const selectedEncounterToLoad = ref(null);
const isLoadingExisting = ref(false);

async function fetchExistingEncounters() {
    isLoadingExisting.value = true;
    try {
        const response = await fetch('http://localhost:8080/api/encounters'); // Bestehenden Endpunkt nutzen
        if (!response.ok) throw new Error('Failed to fetch');
        existingEncounters.value = await response.json();
    } catch (err) {
        console.error("Error fetching existing encounters:", err);
        // Optional: Fehlermeldung anzeigen
    } finally {
        isLoadingExisting.value = false;
    }
}

// Wenn ein Encounter zum Laden ausgewählt wird, sende das Event
watch(selectedEncounterToLoad, (newId) => {
    if (newId) {
        emit('load-encounter', newId);
        // Setze Auswahl zurück nach dem Senden, damit man neu wählen kann
        // Wichtig: Timeout, damit der Parent Zeit hat, die Props zu aktualisieren,
        // bevor wir hier den lokalen Zustand (der auf Props hört) potenziell überschreiben.
        setTimeout(() => { selectedEncounterToLoad.value = null; }, 100);
    }
});

// Lade existierende Encounter beim Mounten
onMounted(() => {
    fetchExistingEncounters();
});

</script>

<template>
    <v-card variant="outlined">
        <v-card-title>Encounter Details</v-card-title>
        <v-card-text>
            <!-- Laden existierender Encounter -->
             <v-autocomplete
                 v-model="selectedEncounterToLoad"
                 :items="existingEncounters"
                 item-title="name"
                 item-value="id"
                 label="Load Existing Encounter"
                 :loading="isLoadingExisting || loading" 
                 :disabled="isLoadingExisting || loading"
                 placeholder="Select to load..."
                 clearable
                 variant="outlined"
                 density="compact"
                 class="mb-4"
                 hide-details="auto"
             />
              <v-alert v-if="error" type="warning" density="compact" class="mb-4">
                  Error loading encounter: {{ error }}
              </v-alert>

            <!-- Eingabefelder -->
            <v-text-field
                :model-value="name"
                @update:model-value="$emit('update:name', $event)"
                label="Encounter Name"
                variant="outlined"
                density="compact"
                class="mb-4"
                hide-details
            />
             <v-textarea
                :model-value="description"
                @update:model-value="$emit('update:description', $event)"
                label="Description"
                variant="outlined"
                density="compact"
                rows="3"
                class="mb-4"
                hide-details
             />

            <!-- Party Info -->
            <v-row dense>
                <v-col cols="6">
                     <v-text-field
                        type="number"
                        :model-value="partySize"
                        @update:model-value="$emit('update:partySize', parseInt($event, 10) || 1)"
                        label="Party Size"
                        min="1"
                        variant="outlined"
                        density="compact"
                        hide-details
                     />
                </v-col>
                 <v-col cols="6">
                     <v-text-field
                        type="number"
                        :model-value="partyLevel"
                        @update:model-value="$emit('update:partyLevel', parseInt($event, 10) || 1)"
                        label="Avg. Party Level"
                        min="1"
                        max="20"
                        variant="outlined"
                        density="compact"
                        hide-details
                     />
                </v-col>
            </v-row>
             <v-btn @click="$emit('clear-encounter')" small class="mt-4">
                Clear / New Encounter
             </v-btn>
        </v-card-text>
    </v-card>
</template>

<style scoped>
/* Stile */
</style>