<script setup>
// Importiere 'ref' von Vue, um reaktive Daten zu erstellen
import { ref } from 'vue';

// Importiere die Komponenten für die einzelnen Tabs (erstmal Platzhalter)
// Diese Dateien musst du später noch erstellen im 'src/components' Ordner
// Beispiel: import Startseite from './components/Startseite.vue'; 
// Beispiel: import CombatTracker from './components/CombatTracker.vue';

// Definiere eine reaktive Variable, die speichert, welcher Tab gerade aktiv ist.
// 'ref' macht die Variable reaktiv, d.h. wenn sich ihr Wert ändert,
// wird die UI automatisch aktualisiert. Wir starten mit dem ersten Tab (Index 0).
const activeTab = ref(null); // Startet ohne ausgewählten Tab, oder setze einen Default z.B. 'tab-start'

// Liste der Tabs, die wir anzeigen wollen
const tabs = [
  { id: 'tab-start', title: 'Startseite', component: null /* Später: Startseite */ },
  { id: 'tab-combat', title: 'Combat Tracker', component: null /* Später: CombatTracker */ },
  { id: 'tab-encounter', title: 'Encounter Creation', component: null },
  { id: 'tab-dmscreen', title: 'DM Screen', component: null },
  { id: 'tab-monster', title: 'Monster Creator', component: null },
  { id: 'tab-maps', title: 'Maps', component: null },
  { id: 'tab-settings', title: 'Settings', component: null },
];
</script>

<template>
  <!-- v-app ist die Haupt-Container-Komponente von Vuetify. Jede Vuetify-Anwendung braucht sie. -->
  <v-app>
    <!-- v-app-bar erstellt die obere Leiste (wie eine Navigationsleiste) -->
    <v-app-bar app color="primary">
      <!-- Der Titel der App (optional) -->
      <v-app-bar-title>D&D Helper</v-app-bar-title>

      <template v-slot:extension>
        <!-- v-tabs ist die Komponente für die Tabs -->
        <v-tabs
          v-model="activeTab" 
          align-tabs="center" 
          centered 
          grow 
          bg-color="primary"
        >
          <!-- 'v-model="activeTab"' bindet den aktiven Tab an unsere reaktive Variable. -->
          <!-- 'align-tabs="center"' zentriert die Tabs in der Leiste. -->
          <!-- 'centered' ist ähnlich wie align-tabs, für ältere Vuetify-Versionen oder andere Effekte -->
          <!-- 'grow' lässt die Tabs den verfügbaren Platz einnehmen -->
          
          <!-- v-for ist eine Vue-Direktive, um über eine Liste (unsere 'tabs') zu iterieren -->
          <!-- Für jeden Eintrag im 'tabs'-Array wird ein v-tab erstellt. -->
          <v-tab 
            v-for="tab in tabs" 
            :key="tab.id" 
            :value="tab.id" 
          >
            <!-- ':key' ist wichtig für Vue, um die Elemente effizient zu verwalten. -->
            <!-- ':value' setzt den Wert des Tabs, der in 'activeTab' gespeichert wird, wenn dieser Tab geklickt wird. -->
            {{ tab.title }} <!-- Zeigt den Titel des Tabs an -->
          </v-tab>
        </v-tabs>
      </template>
    </v-app-bar>

    <!-- v-main ist der Bereich, in dem der Hauptinhalt der Seite angezeigt wird -->
    <v-main>
      <!-- v-container sorgt für etwas Abstand zum Rand und zentriert den Inhalt (optional) -->
      <v-container fluid> 
        <!-- v-window zeigt den Inhalt des jeweils aktiven Tabs an -->
        <!-- 'v-model="activeTab"' verbindet dieses Fenster mit dem aktiven Tab. -->
        <v-window v-model="activeTab">
          <!-- v-window-item wird für jeden möglichen Tab-Inhalt benötigt -->
          <v-window-item 
            v-for="tab in tabs" 
            :key="tab.id" 
            :value="tab.id"
            transition="fade-transition" 
            reverse-transition="fade-transition"
          >
            <!-- ':value' muss mit dem 'value' des entsprechenden v-tab übereinstimmen. -->

            <!-- Hier wird der Inhalt des Tabs angezeigt -->
            <!-- Später werden hier die importierten Komponenten eingefügt -->
            <!-- Beispiel: <component :is="tab.component" v-if="tab.component" /> -->
            <v-card flat>
              <v-card-text>
                <h2>{{ tab.title }}</h2>
                <p>Inhalt für {{ tab.title }} kommt hier hin.</p>
                
                <!-- Beispiel: Status vom Backend abfragen (nur für den ersten Tab) -->
                <div v-if="tab.id === 'tab-start'">
                  <p>Hier könnte der Status vom Backend angezeigt werden.</p>
                  <!-- Wir fügen später eine Komponente hinzu, die /api/status abfragt -->
                </div>

              </v-card-text>
            </v-card>

          </v-window-item>
        </v-window>
      </v-container>
    </v-main>
  </v-app>
</template>

<style scoped>
/* Hier könntest du spezifische Stile für App.vue hinzufügen */
/* 'scoped' bedeutet, dass diese Stile nur für diese Komponente gelten */
.v-main {
  padding-top: 48px; /* Füge Padding hinzu, um Platz für die Tab-Extension zu schaffen */
}
</style>