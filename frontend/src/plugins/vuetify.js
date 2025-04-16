// frontend/src/plugins/vuetify.js

// Styles
import '@mdi/font/css/materialdesignicons.css' // Icon Font
import 'vuetify/styles' // Vuetify Basis-Styles

// Composables (Funktionen von Vuetify)
import { createVuetify } from 'vuetify'

// https://vuetifyjs.com/en/introduction/why-vuetify/#feature-guides
export default createVuetify({
  // Hier könntest du globale Konfigurationen vornehmen, z.B. ein dunkles Theme:
  // theme: {
  //   defaultTheme: 'dark'
  // }
})