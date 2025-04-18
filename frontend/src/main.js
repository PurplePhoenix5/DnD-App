// Importiere die Basis-CSS-Datei von Vue (falls noch nicht da)
import './assets/main.css' 

// Importiere createApp von Vue
import { createApp } from 'vue'

// Importiere die Haupt-App-Komponente
import App from './App.vue'

// --- Vuetify Konfiguration START ---
// Importiere benötigte Funktionen von Vuetify
import 'vuetify/styles' // Importiert die Basis-CSS-Stile von Vuetify
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components' // Importiert alle Vuetify-Komponenten
import * as directives from 'vuetify/directives' // Importiert Vuetify-Direktiven (z.B. für Tooltips, Ripples)
import '@mdi/font/css/materialdesignicons.css' // Importiert die Icon-Bibliothek (optional aber empfohlen)

// Erstelle die Vuetify-Instanz
const vuetify = createVuetify({
  components, // Registriert alle importierten Komponenten
  directives, // Registriert alle importierten Direktiven
  // Hier könntest du globale Konfigurationen vornehmen, z.B. Theme (dark/light)
  theme: {
    defaultTheme: 'dark' 
  }
})
// --- Vuetify Konfiguration ENDE ---

// Erstelle die Vue App Instanz
const app = createApp(App)

// Nutze Vuetify in der App
app.use(vuetify) 

// Hänge die App an das HTML-Element mit der ID 'app' in index.html
app.mount('#app') 