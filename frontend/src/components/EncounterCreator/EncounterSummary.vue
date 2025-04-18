<script setup>
import { computed } from 'vue';

const props = defineProps({
    difficulty: { type: String, default: 'N/A'},
    totalXP: { type: Number, default: 0 },
    adjustedXP: { type: Number, default: 0 },
    progress: { type: Number, default: 0 }, // Fortschritt 0-100
    isSaving: { type: Boolean, default: false },
    isDeleting: { type: Boolean, default: false },
    canDelete: { type: Boolean, default: false } // Ob der Lösch-Button aktiv sein soll
});

const emit = defineEmits(['save', 'delete']);

// Farbe für Schwierigkeit und Fortschrittsbalken
const difficultyColor = computed(() => {
  switch (props.difficulty?.toLowerCase()) {
    case 'trivial': return 'grey';
    case 'easy': return 'success';
    case 'medium': return 'info';
    case 'hard': return 'warning';
    case 'deadly': return 'error';
    default: return 'grey-darken-1';
  }
});
</script>

<template>
    <v-card variant="tonal" class="mt-4">
        <v-card-title>Encounter Summary</v-card-title>
        <v-card-text>
            <div class="d-flex justify-space-between mb-2">
                <span>Difficulty:</span>
                <v-chip :color="difficultyColor" size="small" label>{{ difficulty }}</v-chip>
            </div>
            <div class="d-flex justify-space-between mb-2">
                <span>Total XP:</span>
                <span>{{ totalXP.toLocaleString() }} XP</span>
            </div>
             <div class="d-flex justify-space-between mb-3">
                <span>Adjusted XP (for difficulty):</span>
                <span>{{ adjustedXP.toLocaleString() }} XP</span>
            </div>

            <!-- Fortschrittsbalken -->
            <v-tooltip location="top">
                 <template v-slot:activator="{ props: tooltipProps }">
                     <v-progress-linear
                         v-bind="tooltipProps"
                         :model-value="progress"
                         :color="difficultyColor"
                         height="8"
                         rounded
                         class="mb-4"
                     ></v-progress-linear>
                 </template>
                 <span>{{ progress.toFixed(0) }}% towards next difficulty threshold</span>
            </v-tooltip>


            <!-- Buttons -->
             <div class="d-flex justify-space-between">
                 <v-btn
                    color="error"
                    @click="$emit('delete')"
                    :loading="isDeleting"
                    :disabled="isSaving || !canDelete"
                    prepend-icon="mdi-delete"
                 >
                    Delete
                 </v-btn>
                 <v-btn
                    color="primary"
                    @click="$emit('save')"
                    :loading="isSaving"
                    :disabled="isDeleting"
                    prepend-icon="mdi-content-save"
                 >
                    Save
                 </v-btn>
             </div>

        </v-card-text>
    </v-card>
</template>

<style scoped>
/* Stile */
</style>