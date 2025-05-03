<!-- src/components/Auth/LoginForm.vue -->
<script setup>
import { ref, onMounted } from 'vue'; // Füge onMounted hinzu
import authStore from '../../store/auth.js';

const username = ref(null); // Initialisiere mit null für Select
const password = ref('');
const showPassword = ref(false);
const userList = ref([]); // Ref für die Benutzerliste
const isLoadingUsers = ref(false); // Ladezustand für Benutzerliste

// Zugriff auf den schreibgeschützten State und die Aktionen
const { userSession, login } = authStore;

// Funktion zum Laden der Benutzerliste
async function fetchUserList() {
    isLoadingUsers.value = true;
    userList.value = []; // Liste leeren
    try {
        const response = await fetch('http://localhost:8080/api/users/list');
        if (!response.ok) {
            throw new Error(`HTTP error fetching user list! Status: ${response.status}`);
        }
        const data = await response.json();
        // Erwarte ein Array von Strings
        if (Array.isArray(data)) {
             userList.value = data;
             console.log("User list loaded:", userList.value);
        } else {
             throw new Error("Invalid data format received for user list.");
        }
    } catch (err) {
        console.error("Failed to load user list:", err);
        // Optional: Fehlermeldung im UI anzeigen
    } finally {
        isLoadingUsers.value = false;
    }
}

// Lade die Benutzerliste, wenn die Komponente gemountet wird
onMounted(() => {
    fetchUserList();
});


async function handleLogin() {
    if (username.value && password.value) { // Stelle sicher, dass ein Username ausgewählt ist
        await login(username.value, password.value);
    } else {
         // Optional: Fehlermeldung, dass Username ausgewählt werden muss
         authStore.userSession.error = "Please select a username.";
    }
}
</script>

<template>
    <v-container fluid fill-height>
      <v-row align="center" justify="center">
        <v-col cols="12" sm="8" md="4">
          <v-card class="elevation-12" variant="tonal">
            <v-toolbar color="primary" dark flat>
              <v-toolbar-title>Login</v-toolbar-title>
            </v-toolbar>
            <v-card-text>
              <v-form @submit.prevent="handleLogin">
                <!-- === Geändert: v-select statt v-text-field === -->
                <v-select
                  label="Username"
                  v-model="username"
                  :items="userList"
                  :loading="isLoadingUsers"
                  :disabled="isLoadingUsers || userSession.loading"
                  prepend-icon="mdi-account"
                  required
                  clearable
                  no-data-text="Loading users or none found..."
                  density="compact"
                  variant="outlined"
                  class="mb-4"
                ></v-select>
                <!-- ============================================ -->

                <v-text-field
                  label="Password"
                  v-model="password"
                  prepend-icon="mdi-lock"
                  :append-inner-icon="showPassword ? 'mdi-eye' : 'mdi-eye-off'"
                  :type="showPassword ? 'text' : 'password'"
                  @click:append-inner="showPassword = !showPassword"
                  required
                  :disabled="userSession.loading"
                  density="compact"
                  variant="outlined"
                ></v-text-field>

                <v-alert v-if="userSession.error" type="error" density="compact" class="my-4">
                  {{ userSession.error }}
                </v-alert>

              </v-form>
            </v-card-text>
            <v-card-actions>
              <v-spacer></v-spacer>
              <v-btn
                color="primary"
                @click="handleLogin"
                :loading="userSession.loading"
                :disabled="userSession.loading || !username || !password"
              >
                Login
              </v-btn>
            </v-card-actions>
          </v-card>
        </v-col>
      </v-row>
    </v-container>
</template>

<style scoped>
.v-container {
    /* Mindesthöhe, um die Zentrierung auch bei wenig Inhalt zu gewährleisten */
    min-height: 80vh;
    display: flex;
}
</style>