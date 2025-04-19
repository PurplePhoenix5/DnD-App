<!-- frontend/src/components/MonsterCreator/MonsterLoader.vue -->
<script setup>
import { ref, onMounted, watch } from 'vue';

// Event zum Melden, welches Monster geladen werden soll
const emit = defineEmits([
    'load-monster',
    'update:style',   // Event für Stil-Änderung (v-model:style)
    'update:columns'  // Event für Spalten-Änderung (v-model:columns)
]);

// Daten für die Monster-Suche
const existingMonsters = ref([]); // Format: [{ id: '...', name: '...' }]
const selectedMonsterToLoad = ref(null);
const isLoadingMonsters = ref(false);
const loadError = ref(null);

// === NEU: Lokale Refs für die Toggles (verbunden mit Props über Watcher) ===
const selectedStyle = ref(props.style);
const selectedColumns = ref(props.columns);

// Daten für das Stat-Referenz-Dropdown (Platzhalter)
const statReferenceItems = ref([
    { title: 'SRD Average', value: 'srd_avg' },
    { title: 'Manual Input', value: 'manual' },
    // Weitere Optionen könnten hier später hinzukommen
]);
const selectedStatReference = ref('srd_avg'); // Default-Wert

// Methode zum Laden der existierenden Monsterliste
async function fetchExistingMonsters() {
    isLoadingMonsters.value = true;
    loadError.value = null;
    try {
        // Nutze den Summary-Endpunkt, den wir schon haben
        const response = await fetch('http://localhost:8080/api/monsters/summary');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        // Sortiere die Liste alphabetisch für die Anzeige im Dropdown
        const data = await response.json();
        existingMonsters.value = data.sort((a, b) => a.name.localeCompare(b.name));

    } catch (err) {
        console.error("Error fetching existing monster list:", err);
        loadError.value = "Failed to load monster list.";
    } finally {
        isLoadingMonsters.value = false;
    }
}

// Lade Monsterliste beim Mounten
onMounted(() => {
    fetchExistingMonsters();
});

// Beobachte die Auswahl im Autocomplete-Feld
watch(selectedMonsterToLoad, (newId) => {
    if (newId) {
        console.log('MonsterLoader: Emitting load-monster with ID:', newId);
        emit('load-monster', newId);
        // Optional: Auswahl zurücksetzen nach dem Laden?
        // selectedMonsterToLoad.value = null; // Oder im Parent nach erfolgreichem Laden
    }
});

watch(selectedStyle, (newStyle) => {
    emit('update:style', newStyle);
});

watch(selectedColumns, (newColumns) => {
    emit('update:columns', newColumns);
});


watch(() => props.style, (newVal) => selectedStyle.value = newVal );
watch(() => props.columns, (newVal) => selectedColumns.value = newVal );


</script>

<template>
    <v-card variant="tonal" class="mb-4">
        <v-card-text>
            <v-row align="center" justify="center" dense>
                <!-- Spalte für das Suchfeld -->
                <v-col cols="12" sm="6" md="5">
                    <v-autocomplete
                        v-model="selectedMonsterToLoad"
                        :items="existingMonsters"
                        item-title="name"
                        item-value="id"
                        label="Load Existing Monster"
                        :loading="isLoadingMonsters"
                        :disabled="isLoadingMonsters"
                        placeholder="Search and select..."
                        clearable
                        variant="solo-filled" 
                        density="compact"
                        hide-details="auto"
                        no-data-text="No monsters found or loading..."
                        prepend-inner-icon="mdi-magnify"
                    />
                     <v-alert v-if="loadError" type="warning" density="compact" class="mt-1">
                        {{ loadError }}
                     </v-alert>
                </v-col>

                <!-- Spalte für das Stat-Referenz-Dropdown -->
                <v-col cols="12" sm="6" md="4">
                     <v-select
                        v-model="selectedStatReference"
                        :items="statReferenceItems"
                        item-title="title"
                        item-value="value"
                        label="Stat Reference (WIP)"
                        variant="solo-filled"
                        density="compact"
                        hide-details
                        disabled 
                     ></v-select>
                     <div class="text-caption text-disabled text-center mt-1">(Not implemented yet)</div>
                </v-col>
            </v-row>
            <v-row align="center" justify="center" dense>
                 <!-- Spalte für Style Auswahl -->
                 <v-col cols="auto">
                    <span class="text-caption mr-2">Style:</span>
                    <v-btn-toggle
                        v-model="selectedStyle"
                        variant="outlined"
                        divided
                        density="compact"
                        mandatory 
                    >
                        <v-btn value="2014">2014</v-btn>
                        <v-btn value="2024">2024</v-btn>
                    </v-btn-toggle>
                 </v-col>
                 <!-- Spalte für Spalten Auswahl -->
                  <v-col cols="auto">
                     <span class="text-caption mr-2">Columns:</span>
                     <v-btn-toggle
                         v-model="selectedColumns"
                         variant="outlined"
                         divided
                         density="compact"
                         mandatory 
                     >
                         <v-btn :value="1" icon="mdi-view-sequential-outline"></v-btn>
                         <v-btn :value="2" icon="mdi-view-parallel-outline"></v-btn>
                     </v-btn-toggle>
                 </v-col>
            </v-row>
        </v-card-text>
    </v-card>
</template>

<style scoped>
/* Optional: Stile anpassen */
</style>