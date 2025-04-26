// Importiere die Basis-CSS-Datei von Vue
import './assets/main.css'

// Importiere createApp von Vue
import { createApp } from 'vue'

// Importiere die Haupt-App-Komponente
import App from './App.vue'

// --- Vuetify Konfiguration START ---
import 'vuetify/styles'
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'
import '@mdi/font/css/materialdesignicons.css'

// Erstelle die Vuetify-Instanz mit Theme-Konfiguration
const vuetify = createVuetify({
  components: {
    ...components,
    
  },
  directives,
  
  // === HIER KOMMT DIE THEME-KONFIGURATION ===
  theme: {
    // 1. Setze das Standard-Theme auf 'dark'
    defaultTheme: 'dark',
    themes: {
      // 2. Definiere dein 'dark' Theme (oder passe es an)
      dark: {
        dark: true, // Wichtig, um Vuetify mitzuteilen, dass dies ein dunkles Theme ist
        colors: {
          // === Hier überschreiben wir die Standardfarben ===
          primary: '#651FFF', // Deine gewünschte Akzentfarbe
          background: '#181818', // Typischer dunkler Hintergrund (kannst du anpassen)
          surface: '#212121',    // Typische dunkle Oberfläche für Karten etc. (kannst du anpassen)
          // Sekundär-, Fehler-, Info-, Erfolgs-, Warnfarben werden von Vuetify's dark theme geerbt,
          // außer du überschreibst sie hier auch explizit:
          // secondary: '#...',
          // accent: '#...', // Oft ähnlich wie primary oder eine Kontrastfarbe
          // error: '#B00020',
          // info: '#2196F3',
          // success: '#4CAF50',
          // warning: '#FB8C00',
        }
      },
      // 3. Optional: Definiere auch ein 'light' Theme, falls du mal wechseln möchtest
      light: {
        dark: false,
        colors: {
           primary: '#651FFF', // Dieselbe Primärfarbe auch im hellen Theme?
           background: '#FFFFFF',
           surface: '#FFFFFF',
          // secondary: '#...',
          // ... andere helle Farben ...
        }
      }
    }
  }
  // ==========================================
})
// --- Vuetify Konfiguration ENDE ---

// Erstelle die Vue App Instanz
const app = createApp(App)

// Nutze Vuetify in der App
app.use(vuetify)

// Hänge die App an das HTML-Element mit der ID 'app' in index.html
app.mount('#app')