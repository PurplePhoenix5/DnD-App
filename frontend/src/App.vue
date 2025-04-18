<script setup>
import { ref } from 'vue';
import CombatTracker from './components/CombatTracker/CombatTracker.vue';
import StatusDisplay from './components/StatusDisplay.vue';

const activeTab = ref('tab-start');
const tabs = ref([
  { id: 'tab-start', title: 'Startseite', component: StatusDisplay },
  { id: 'tab-combat', title: 'Combat Tracker', component: CombatTracker },
  { id: 'tab-encounter', title: 'Encounter Creation', component: null },
  { id: 'tab-dmscreen', title: 'DM Screen', component: null },
  { id: 'tab-monster', title: 'Monster Creator', component: null },
  { id: 'tab-maps', title: 'Maps', component: null },
  { id: 'tab-settings', title: 'Settings', component: null },
]);
</script>

<template>
  <v-app>
    <v-app-bar app color="darken">
      <template v-slot:extension>
        <v-tabs
          v-model="activeTab"
          align-tabs="center"
          centered
          grow
          color="deep-purple-accent-4"
          bg-color="transparent"
        >
          <v-tab
            v-for="tab in tabs"
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
            v-for="tab in tabs"
            :key="tab.id"
            :value="tab.id"
            transition="fade-transition"
            reverse-transition="fade-transition"
            v-slot:default 
            eager
          >
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
  </v-app>
</template>

<style scoped>
/* Stile hier, falls nötig */
</style>