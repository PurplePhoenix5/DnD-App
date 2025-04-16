// frontend/src/router/index.js
import { createRouter, createWebHistory } from 'vue-router';

// Importiere deine View-Komponenten
import CombatTrackerView from '../views/CombatTrackerView.vue';
import EncounterBuilderView from '../views/EncounterBuilderView.vue';
import DmScreenView from '../views/DmScreenView.vue';
import MapView from '../views/MapView.vue';
import Tab5View from '../views/Tab5View.vue';
import Tab6View from '../views/Tab6View.vue';
import Tab7View from '../views/Tab7View.vue';
import Tab8View from '../views/Tab8View.vue';


const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL), // Standard-History-Modus für SPAs
  routes: [
    {
      path: '/', // Die URL für diesen Tab
      name: 'combat-tracker', // Ein eindeutiger Name für die Route
      component: CombatTrackerView // Die Vue-Komponente, die angezeigt werden soll
    },
    {
      path: '/encounter-builder',
      name: 'encounter-builder',
      component: EncounterBuilderView
    },
    {
      path: '/dm-screen',
      name: 'dm-screen',
      component: DmScreenView
    },
    {
      path: '/maps',
      name: 'maps',
      component: MapView
    },
    {
      path: '/tab5', 
      name: 'tab5',
      component: Tab5View
    },
    {
      path: '/tab6',
      name: 'tab6',
      component: Tab6View
    },
    {
      path: '/tab7', 
      name: 'tab7',
      component: Tab7View
    },
    {
      path: '/tab8', 
      name: 'tab8',
      component: Tab8View
    },
  ]
});

export default router;