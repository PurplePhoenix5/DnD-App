// frontend/src/main.js

import { createApp } from 'vue'
import App from './App.vue'
import router from './router' // Unser Vue Router

// Importiere das Vuetify Plugin
import vuetify from './plugins/vuetify'

// Lade die Vuetify Fonts (optional, aber oft empfohlen)
import { loadFonts } from './plugins/webfontloader'

loadFonts() // Lade die Schriften

const app = createApp(App)

app.use(router) // Sage Vue, dass es den Router benutzen soll
app.use(vuetify) // Sage Vue, dass es Vuetify benutzen soll

app.mount('#app') // Starte die App im HTML-Element mit der ID 'app'