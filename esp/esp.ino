/*
 *
 * GUARDARE IL README.md PER LA DOCUMENTAZIONE
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Credenziali di rete
const char* ssid = "SSID";
const char* pwd = "PASSWORD";

// pagina da renderizzare alla richiesta get
// !!! è normale che venga visualizzata bianca, la tokenizzazione non funziona su stringhe lunghe. !!!
const char* htmlPage = "<!doctype html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><link rel=\"icon\" type=\"image/png\" href=\"https://i.ibb.co/vB1mKmF/logo-Pascal.png\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><title>ESP32 Chess WebServer</title><script type=\"module\" crossorigin src=\"http://87.6.244.237:12432/index.js\"></script><link rel=\"stylesheet\" crossorigin href=\"http://87.6.244.237:12432/index.css\"></head><body><div id=\"root\"></div></body></html>";

// WebServer
AsyncWebServer server(80);
// Handler WebSocket
AsyncWebSocket ws("/ws");

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Array che contiene gli id dei giocatori, definizione degli index.
#define BLACK 0
#define WHITE 1
int16_t players[2] = {-1,-1};

// Inizializzazione del WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA); // Station Mode (classica)
  WiFi.begin(ssid, pwd);
  Serial.printf("Connettendo alla rete '%s'...", ssid);
  while (WiFi.status() != WL_CONNECTED) { // printo punti a intervalli di 500ms finché non avviene la connessione
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\n%s\n",WiFi.localIP().toString().c_str()); // printo l'ip locale
}

#define getOpponent(id) ((id == players[WHITE]) ? players[BLACK] : players[WHITE])
void handleWebSocketMessage(uint32_t clientId, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo * info = (AwsFrameInfo*)arg; // faccio un cast dell'argomento in un AwsFrameInfo
  if (info->final && info->index == 0 && info->len == len) { // se il buffer del messaggio è pronto
    if (info->opcode == WS_TEXT) { // se l'opcode del messaggio è TEXT
      data[len] = 0; // aggiungo il terminatore di stringa in fondo al buffer
      Serial.printf("Client #%u ha inviato: %s\n", clientId, (char*)data); // printo il mesaggio
      String FENString = String((char*)data); // converto il buffer in una stringa
      ws.text(getOpponent(clientId), FENString); // invio la stringa al client avversario
      /*
        GESTIONE DELLA STRINGA FEN -> Invio all'S7
      */

    }
  }
}

// Gestione degli eventi del server WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type) {
    //////////////////////////////////////////////////////
    case WS_EVT_CONNECT: // Connessione di un client
      Serial.printf("Client WebSocket #%u in connessione da %s... ", client->id(), client->remoteIP().toString().c_str());

      // -- Memorizzazione dell'ID del client, assegnazione del ruolo
      if (players[WHITE] >= 0 && players[BLACK] >= 0) { // Controllo che non ci siano già due dispositivi in gioco
        client->close(1000, "Sono già collegati due dispositivi.");
        Serial.printf("Rifiutato (troppi clients).\n");
        return;
      }

      if (players[WHITE] >= 0) { // se il ruolo WHITE è già assegnato
        players[BLACK] = (int16_t)client->id(); // assegno il ruolo BLACK all'id del client
        client->text("BLACK"); // invio al client il ruolo assegnato
      }
      else { // altrimenti
        players[WHITE] = (int16_t)client->id();
        client->text("WHITE");
      }
      // --

      Serial.printf("Connesso.\n");
      break;
    //////////////////////////////////////////////////////
    case WS_EVT_DISCONNECT: // Disconnessione di un client
      Serial.printf("Client #%u in disconnessione... ", client->id());

      // -- Rimozione del ruolo
      if (players[WHITE] == client->id()) players[WHITE] = -1;
      if (players[BLACK] == client->id()) players[BLACK] = -1;
      // --

      Serial.printf("Disconnesso.\n");
      break;
    //////////////////////////////////////////////////////
    case WS_EVT_DATA: // Dati in arrivo
      handleWebSocketMessage(client->id(), arg, data, len);
      break;
    //////////////////////////////////////////////////////
    case WS_EVT_PONG: // Evento di Pong
    case WS_EVT_ERROR: // Evento di errore
      break;
  }
}

// Inizializza il server WebSocket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws); // aggiungo il websocket agli handlers del web server
}

void setup() {
  Serial.begin(115200); // inizializzo il seriale con un BAUD Rate di 115200
  delay(1000); // aspetto che la connessione seriale si inizializzi
  initWiFi();
  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) { // imposto una richiesta HTTP GET in modo che ritorni la pagina html
    request->send(200,"text/html",htmlPage); // rispondo con un codice HTTP di 200 (OK) e ritorno la pagina
  });

  server.begin(); // starto il webserver
}

void loop() {
  ws.cleanupClients(2); // Limito i Clients a 2
}
