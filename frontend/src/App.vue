<template>
  <!-- v-app ist die Wurzelkomponente für Vuetify -->
  <v-app>
    <!-- v-app-bar erstellt die obere Leiste -->
    <v-app-bar app color="primary" dark>
      <!-- Optional: Ein Icon vor dem Titel -->
      <v-app-bar-nav-icon @click="drawer = !drawer"></v-app-bar-nav-icon> <!-- Beispiel für einen Drawer-Button -->
      <v-toolbar-title>DnD Helper</v-toolbar-title>

      <v-spacer></v-spacer> <!-- Schiebt die Tabs nach rechts -->

      <!-- v-tabs enthält die einzelnen Tabs.
           Wir verstecken sie auf kleinen Bildschirmen (xs) und zeigen stattdessen den Drawer -->
      <v-tabs align-with-title class="hidden-xs-only">
        <!-- Iterieren über das 'tabs'-Array -->
        <v-tab
          v-for="tab in tabs"
          :key="tab.route"
          :to="tab.route"
        >
          <!-- Optional: Icon hinzufügen -->
          <v-icon start v-if="tab.icon">{{ tab.icon }}</v-icon>
          {{ tab.name }}
        </v-tab>
      </v-tabs>
    </v-app-bar>

    <!-- Optional: Navigation Drawer für kleinere Bildschirme -->
    <!-- Zeige Drawer nur auf xs -->
    <v-navigation-drawer
      v-model="drawer"
      app
      temporary  
      class="hidden-sm-and-up"
    >
      <v-list nav dense>
        <v-list-item
          v-for="tab in tabs"
          :key="tab.route"
          :to="tab.route"
          link
        >
          <template v-slot:prepend>
            <v-icon v-if="tab.icon">{{ tab.icon }}</v-icon>
          </template>
          <v-list-item-title>{{ tab.name }}</v-list-item-title>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>

    <!-- v-main ist der Bereich, in dem der Inhalt des aktiven Tabs angezeigt wird -->
    <v-main>
      <!-- router-view lädt die passende View für die aktuelle URL -->
      <router-view v-slot="{ Component }">
          <v-fade-transition mode="out-in">
               <component :is="Component" />
          </v-fade-transition>
      </router-view>
    </v-main>
  </v-app>
</template>

<script setup>
import { ref } from 'vue';

// Ref für den Drawer-Status (öffnen/schließen)
const drawer = ref(false);

// Definiere die Tabs mit Namen, Routen und optionalen Icons (Material Design Icons)
// Beispiele für Icons: https://materialdesignicons.com/
const tabs = ref([
  { name: 'Combat Tracker', route: '/'},
  { name: 'Encounter Builder', route: '/encounter-builder'},
  { name: 'DM Screen', route: '/dm-screen'},
  { name: 'Maps', route: '/maps'},
  { name: 'Tab 5', route: '/tab5'}, // Beispiel-Icon
  { name: 'Tab 6', route: '/tab6'}, // Beispiel-Icon
  { name: 'Tab 7', route: '/tab7'}, // Beispiel-Icon
  { name: 'Tab 8', route: '/tab8'}, // Beispiel-Icon
]);
</script>

<style scoped>
/* CSS-Regeln, die nur für diese Komponente gelten. */
/* Z.B. Übergangsanimationen etwas anpassen, falls gewünscht */
.v-main {
  /* Optional: Fügt etwas Padding hinzu, wenn gewünscht */
  padding: 16px;
}
</style>