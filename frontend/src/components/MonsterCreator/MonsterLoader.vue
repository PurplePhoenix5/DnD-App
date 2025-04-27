<!-- frontend/src/components/MonsterCreator/MonsterLoader.vue -->
<script setup>
import { ref, onMounted, watch, computed } from 'vue';

const props = defineProps({
    style: { type: String, default: '2024' },
    columns: { type: Number, default: 1 },
    monsterName: { type: String, default: '' },
    monsterCR: { type: [Number, null], default: null },
    isSaving: { type: Boolean, default: false },
    isDeleting: { type: Boolean, default: false }
});

// Event zum Melden, welches Monster geladen werden soll
const emit = defineEmits([
    'load-monster',
    'update:style',   // Event für Stil-Änderung (v-model:style)
    'update:columns',  // Event für Spalten-Änderung (v-model:columns)
    'save-monster', // Event zum Speichern
    'delete-monster' // Event zum Löschen
    // 'upload-json' // Event für Upload (später)
]);

// Daten für die Monster-Suche
const existingMonsters = ref([]); 
const selectedMonsterToLoad = ref(null);
const isLoadingMonsters = ref(false);
const loadError = ref(null);

// --- Refs für Toggles ---
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

watch(selectedMonsterToLoad, (newId) => {
    // newId sollte hier der Wert von item-value sein (die Monster-ID als String)
    if (newId && typeof newId === 'string') { // Zusätzliche Prüfung auf String
        console.log('MonsterLoader: Emitting load-monster with ID:', newId);
        emit('load-monster', newId);
    } else if (newId) {
         // Fallback, falls doch das Objekt kommt (sollte nicht passieren ohne return-object)
         console.warn('MonsterLoader: selectedMonsterToLoad is an object, emitting id:', newId.id);
         if (newId.id) emit('load-monster', newId.id);
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

const canSaveOrDelete = computed(() => {
    // Speichern/Löschen erlaubt, wenn Name und CR gültig sind
    return !!props.monsterName?.trim() && typeof props.monsterCR === 'number' && props.monsterCR >= 0;
});

function save() {
    if (canSaveOrDelete.value) {
        emit('save-monster');
    }
}

function deleteMonster() {
    // Löschen funktioniert nur, wenn ein Monster *geladen* wurde (selectedMonsterToLoad hat eine ID)
    if (selectedMonsterToLoad.value && canSaveOrDelete.value) { // Zusätzliche Prüfung auf Name/CR des *geladenen* Monsters
        if (confirm(`Are you sure you want to delete monster "${existingMonsters.value.find(m => m.id === selectedMonsterToLoad.value)?.name ?? selectedMonsterToLoad.value}"? This cannot be undone.`)) {
            emit('delete-monster', selectedMonsterToLoad.value); // Sende die ID zum Löschen
        }
    } else {
        alert("Select an existing monster from the list to delete it.");
    }
}

function uploadJson() {
    // Platzhalter für spätere Implementierung
    alert("Upload JSON functionality not implemented yet.");
}

</script>

<template>
    <v-card variant="tonal" class="mb-4">
        <v-card-text>
            <!-- === Zeile 1: Laden & Stat Ref === -->
            <v-row dense class="mb-2">
                <v-col cols="12" md="6"> <!-- Breite angepasst -->
                    <v-autocomplete
                        v-model="selectedMonsterToLoad"
                        :items="existingMonsters"
                        item-title="name"
                        item-value="id"
                        label="Load Existing / Search"
                        :loading="isLoadingMonsters"
                        :disabled="isLoadingMonsters"
                        placeholder="Search or select..."
                        clearable  
                        variant="solo-filled" density="compact" hide-details="auto"
                        no-data-text="No monsters found..." prepend-inner-icon="mdi-magnify"
                    >
                        <!-- Slot, um Item mit complete-Icon anzuzeigen -->
                         <template v-slot:item="{ props: itemProps, item }">
                           <v-list-item v-bind="itemProps" :title="item.raw.name">
                             <template v-slot:prepend>
                               <v-tooltip location="top" :text="item.raw.complete ? 'Complete' : 'Incomplete'">
                                 <template v-slot:activator="{ props: tooltipProps }">
                                   <v-icon
                                     v-bind="tooltipProps"
                                     :icon="item.raw.complete ? 'mdi-check-circle' : 'mdi-circle-edit-outline'"
                                     :color="item.raw.complete ? 'success' : 'warning'"
                                     size="x-small"
                                     class="mr-2"
                                   ></v-icon>
                                 </template>
                               </v-tooltip>
                             </template>
                           </v-list-item>
                         </template>
                    </v-autocomplete>
                     <v-alert v-if="loadError" type="warning" density="compact" class="mt-1"> {{ loadError }} </v-alert>
                </v-col>
                <v-col cols="12" md="6"> <!-- Breite angepasst -->
                     <v-select
                        v-model="selectedStatReference" :items="statReferenceItems"
                        item-title="title" item-value="value" label="Stat Reference (WIP)"
                        variant="solo-filled" density="compact" hide-details disabled
                     />
                </v-col>
            </v-row>

            <!-- === Zeile 2: Aktionen & Layout === -->
            <v-row dense align="center">
                <!-- Save Button -->
                <v-col cols="12" sm="6" md="3">
                     <v-btn
                        @click="save"
                        :disabled="!canSaveOrDelete || isSaving || isDeleting"
                        :loading="isSaving"
                        color="primary" block prepend-icon="mdi-content-save">
                        Save
                     </v-btn>
                </v-col>
                 <!-- Delete Button -->
                 <v-col cols="12" sm="6" md="3">
                     <v-btn
                         @click="deleteMonster"
                         :disabled="!selectedMonsterToLoad || isSaving || isDeleting"
                         :loading="isDeleting"
                         color="error" block prepend-icon="mdi-delete">
                         Delete Loaded
                     </v-btn>
                 </v-col>
                 <!-- Layout/Column Toggle -->
                  <v-col cols="12" sm="6" md="3">
                     <v-btn-toggle
                         v-model="selectedColumns"
                         variant="outlined" divided mandatory density="compact" block
                     >
                         <v-tooltip location="top" text="Single Column">
                             <template v-slot:activator="{props: tooltipProps}">
                                <v-btn :value="1" v-bind="tooltipProps" icon="mdi-view-sequential-outline" class="flex-grow-1"></v-btn>
                             </template>
                         </v-tooltip>
                         <v-tooltip location="top" text="Two Columns">
                               <template v-slot:activator="{props: tooltipProps}">
                                 <!-- Entferne icon="" Prop -->
                                 <v-btn :value="2" v-bind="tooltipProps" class="flex-grow-1">
                                     <!-- Verwende den Default Slot für zwei Icons -->
                                     <div class="d-flex align-center justify-center ga-0"> <!-- ga-0 für keinen Abstand -->
                                         <v-icon icon="mdi-view-sequential-outline" ></v-icon>
                                         <v-icon icon="mdi-view-sequential-outline" ></v-icon>
                                     </div>
                                 </v-btn>
                             </template>
                         </v-tooltip>
                     </v-btn-toggle>
                 </v-col>
                  <!-- Upload Button (Platzhalter) -->
                  <v-col cols="12" sm="6" md="3">
                      <v-btn @click="uploadJson" disabled block prepend-icon="mdi-upload">
                         Upload JSON
                      </v-btn>
                  </v-col>
            </v-row>
        </v-card-text>
    </v-card>
</template>

<style scoped>
.v-btn-toggle {
    display: flex; /* Sorgt dafür, dass Buttons die volle Breite einnehmen */
}
.v-btn-toggle .v-btn {
    flex: 1 1 auto; /* Lässt Buttons gleichmäßig wachsen */
}
</style>