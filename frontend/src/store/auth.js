// src/store/auth.js
import { reactive, readonly } from 'vue';

// Reaktives Objekt für den Benutzerzustand
const userSession = reactive({
    isLoggedIn: false,
    username: null,
    role: null, // 'DM', 'Player' oder null
    error: null,
    loading: false,
});

// Funktion zum Initialisieren aus localStorage
function initAuth() {
    const storedUser = localStorage.getItem('userSession');
    if (storedUser) {
        try {
            const parsedUser = JSON.parse(storedUser);
            if (parsedUser.username && parsedUser.role) {
                userSession.isLoggedIn = true;
                userSession.username = parsedUser.username;
                userSession.role = parsedUser.role;
                 console.log('Auth Store: Session restored from localStorage for', userSession.username);
            } else {
                logout(); // Ungültige Daten im Speicher
            }
        } catch (e) {
            console.error('Auth Store: Failed to parse stored user session', e);
            logout(); // Fehler beim Parsen
        }
    } else {
         console.log('Auth Store: No active session found in localStorage.');
    }
}

// Login Funktion
async function login(username, password) {
    userSession.loading = true;
    userSession.error = null;
    try {
        const response = await fetch('http://localhost:8080/api/login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ username, password }),
        });

        const data = await response.json(); // Versuche immer, JSON zu parsen

        if (!response.ok) {
            // Fehler vom Backend (z.B. 401)
            throw new Error(data.error || `HTTP error! status: ${response.status}`);
        }

        // Login erfolgreich
        userSession.isLoggedIn = true;
        userSession.username = data.username;
        userSession.role = data.role; // Rolle vom Backend übernehmen
        console.log('Auth Store: Login successful for', userSession.username, 'Role:', userSession.role);

        // Speichere Zustand im localStorage
        localStorage.setItem('userSession', JSON.stringify({
            username: userSession.username,
            role: userSession.role,
        }));

        // Optional: Navigiere nach erfolgreichem Login weiter (wenn Vue Router verwendet wird)
        // const router = useRouter(); // Geht nur innerhalb von setup()
        // Stattdessen: Übergebe den Router oder nutze ihn in der Komponente nach dem Login
        return true; // Signalisiert Erfolg

    } catch (err) {
        console.error('Auth Store: Login failed:', err);
        userSession.error = err.message || 'Login failed. Please check credentials.';
        logout(); // Logge User aus bei Fehler
        return false; // Signalisiert Fehler
    } finally {
        userSession.loading = false;
    }
}

// Logout Funktion
function logout() {
    console.log('Auth Store: Logging out user', userSession.username);
    userSession.isLoggedIn = false;
    userSession.username = null;
    userSession.role = null;
    userSession.error = null;
    localStorage.removeItem('userSession');
    // Optional: Weiterleitung zur Login-Seite
    // window.location.reload(); // Einfachste Methode, um sicherzustellen, dass alles zurückgesetzt wird
}

// Initialisiere beim Laden des Moduls
initAuth();

// Exportiere den (readonly) State und die Aktionen
export default {
    userSession: readonly(userSession), // Mache den State von außen readonly
    login,
    logout,
    initAuth, // Exportiere initAuth, falls es manuell getriggert werden muss
};