<!-- src/components/HomeView.vue -->
<script setup>
import authStore from '../store/auth.js';
const { userSession } = authStore;
</script>

<template>
    <v-container>
        <v-card variant="tonal">
            <v-card-title>Welcome!</v-card-title>
            <v-card-text>
                <p>Hello, {{ userSession.username }}!</p>
                <p>Your current role is: <strong>{{ userSession.role }}</strong></p>
                <p>Select a tab to get started.</p>
                <!-- Hier könnte später rollenspezifischer Inhalt hin -->
            </v-card-text>
        </v-card>
    </v-container>
</template>

<style scoped>
/* Optional: Add some styling */
</style>