<!-- App.vue -->
<script setup>
import { ref, computed, watch } from 'vue'; // Importiere watch
import authStore from './store/auth.js'; // Importiere den Auth Store

// Importiere ALLE möglichen Komponenten
import CombatTracker from './components/CombatTracker/CombatTracker.vue';
import EncounterCreator from './components/EncounterCreator/EncounterCreator.vue';
import MonsterCreator from './components/MonsterCreator/MonsterCreator.vue';
import LoginForm from './components/Auth/LoginForm.vue'; // Importiere Login Form
import HomeView from './components/HomeView.vue'; // Importiere Home View

const { userSession, logout } = authStore; // Hole State und Logout-Funktion

const activeTab = ref(null); // Startet ohne aktiven Tab

// Definiere ALLE möglichen Tabs mit ihren Rollen
const allTabs = [
  // Gemeinsame Tabs
  { id: 'tab-start', title: 'Startseite', component: HomeView, roles: ['DM', 'Player', 'Dev'] }, 
  { id: 'tab-settings', title: 'Settings', component: null, roles: ['DM', 'Player', 'Dev'] }, 

  // DM Tabs 
  { id: 'tab-combat', title: 'Combat Tracker', component: CombatTracker, roles: ['DM', 'Dev'] },
  { id: 'tab-encounter', title: 'Encounter Creator', component: EncounterCreator, roles: ['DM', 'Dev'] },
  { id: 'tab-dmscreen', title: 'DM Screen', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-monster', title: 'Monster Creator', component: MonsterCreator, roles: ['DM', 'Dev'] },
  { id: 'tab-maps', title: 'Maps', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-party', title: 'Party Overview', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-class', title: 'Class Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-subclass', title: 'Subclass Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-item', title: 'Item Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-spell', title: 'Spell Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-background', title: 'Background Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-feat', title: 'Feat Creator', component: null, roles: ['DM', 'Dev'] },
  { id: 'tab-species', title: 'Species Creator', component: null, roles: ['DM', 'Dev'] },

  // Player Tabs 
  { id: 'tab-charsheet', title: 'Charakter Sheet', component: null, roles: ['Player', 'Dev'] },
  { id: 'tab-notes', title: 'Notes', component: null, roles: ['Player', 'Dev'] },
  { id: 'tab-foundry', title: 'FoundryVTT', component: null, roles: ['Player', 'Dev'] },
  { id: 'tab-charcreator', title: 'Charakter Creator', component: null, roles: ['Player', 'Dev'] },
];

// Berechnete Eigenschaft, die nur die Tabs für die aktuelle Rolle filtert
const visibleTabs = computed(() => {
    if (!userSession.isLoggedIn || !userSession.role) {
        return []; // Keine Tabs anzeigen, wenn nicht eingeloggt
    }
    // === Dev-Rolle ===
    if (userSession.role === 'Dev') {
        return allTabs; // Dev sieht alle Tabs
    }
    // ======================
    // Filter für DM und Player
    return allTabs.filter(tab => tab.roles.includes(userSession.role));
});

// Setze den aktiven Tab und berücksichtige den gespeicherten Tab
watch(visibleTabs, (newTabs) => {
    if (newTabs.length > 0) {
        const lastTabId = localStorage.getItem('lastActiveTabId');
        // Prüfe, ob der gespeicherte Tab in den *jetzt* sichtbaren Tabs vorhanden ist
        if (lastTabId && newTabs.some(tab => tab.id === lastTabId)) {
            activeTab.value = lastTabId;
             console.log('Restored last active tab:', lastTabId);
        } else if (!activeTab.value || !newTabs.some(tab => tab.id === activeTab.value)) {
             // Wenn kein letzter Tab gespeichert war ODER der aktuelle nicht mehr sichtbar ist,
             // setze den ersten sichtbaren Tab als aktiv
             activeTab.value = newTabs[0].id;
             console.log('Setting first visible tab as active:', activeTab.value);
        }
        // Wenn der aktuelle Tab noch gültig ist, ändere nichts
    } else {
         // Kein Tab mehr sichtbar (nach Logout)
         activeTab.value = null;
         localStorage.removeItem('lastActiveTabId'); // Lösche gespeicherten Tab beim Logout
    }
}, { immediate: true }); // immediate, um initial zu prüfen

// === Speichere aktiven Tab im localStorage ===
watch(activeTab, (newTabId) => {
    if (userSession.isLoggedIn && newTabId) {
         console.log('Saving last active tab:', newTabId);
        localStorage.setItem('lastActiveTabId', newTabId);
    }
});
// =================================================

</script>

<template>
  <v-app>
    <!-- Zeige Login Form, wenn nicht eingeloggt -->
    <LoginForm v-if="!userSession.isLoggedIn" />

    <!-- Zeige Haupt-App, wenn eingeloggt -->
    <template v-else>
      <v-app-bar app color="darken">
         <!-- Logout Button -->
         <template v-slot:append>
              <span class="mr-2 text-caption">Logged in as: {{ userSession.username }} ({{ userSession.role }})</span>
              <v-btn icon @click="logout">
                   <v-tooltip activator="parent" location="bottom">Logout</v-tooltip>
                   <v-icon>mdi-logout</v-icon>
              </v-btn>
         </template>

         <!-- Tabs basierend auf Rolle -->
         <template v-slot:extension>
            <v-tabs
              v-model="activeTab"
              align-tabs="center"
              centered
              grow
              color="deep-purple-accent-4"
              bg-color="transparent"
              v-if="visibleTabs.length > 0"
            >
              <v-tab
                v-for="tab in visibleTabs" 
                :key="tab.id"
                :value="tab.id"
              >
                {{ tab.title }}
              </v-tab>
            </v-tabs>
         </template>
      </v-app-bar>

      <v-main>
         <v-window v-model="activeTab">
            <v-window-item
               v-for="tab in visibleTabs" 
               :key="tab.id"
               :value="tab.id"
               transition="fade-transition"
               reverse-transition="fade-transition"
               eager
             >
               <!-- Verwende den computed visibleTabs Array zum Rendern -->
               <component :is="tab.component" v-if="tab.component" />
               <v-card flat v-else>
                 <v-card-text>
                   <h2>{{ tab.title }}</h2>
                   <p>Content for "{{ tab.title }}" is not implemented yet.</p>
                 </v-card-text>
               </v-card>
             </v-window-item>
         </v-window>
      </v-main>
    </template>

  </v-app>
</template>

<style scoped>
/* Stile hier, falls nötig */
</style>