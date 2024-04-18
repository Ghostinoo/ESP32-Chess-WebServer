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

// Inizializzazione del WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA); // Station Mode (classica)
  WiFi.begin(ssid, pwd);
  Serial.printf("Connettendo alla rete '%s'...", ssid);
  while (WiFi.status() != WL_CONNECTED) { // printo punti a intervalli di 500ms finché non avviene la connessione
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\n%s",WiFi.localIP().toString().c_str()); // printo l'ip locale
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo * info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len) { // se il buffer del messaggio è pronto
    if (info->opcode == WS_TEXT) { // se l'opcode del messaggio è TEXT
      data[len] = 0; // aggiungo il terminatore di stringa
      Serial.println((char*)data); // printo la stringa
    }
  }
}

// Gestione degli eventi del server WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type) {
    case WS_EVT_CONNECT: // Connessione di un client
      Serial.printf("Client WebSocket #%u connesso da %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT: // Disconnessione di un client
      Serial.printf("Client WebSocket #%u disconnesso.\n", client->id());
      break;
    case WS_EVT_DATA: // Dati in arrivo
      handleWebSocketMessage(arg, data, len);
      break;
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
  if ((millis() - lastTime) > timerDelay) {
    ws.textAll("check"); // esempio, ogni timerDelay millisecondi manda un check
    lastTime = millis();
  }
  ws.cleanupClients(2); // Limito i Clients a 2
}
