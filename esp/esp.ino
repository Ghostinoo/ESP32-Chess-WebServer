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
const char* htmlPage = "<!doctype html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><link rel=\"icon\" type=\"image/png\" href=\"http://IP.DEL.PROVIDER/logoPascal.png\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /><title>ESP32 Chess WebServer</title><script type=\"module\" crossorigin src=\"http://IP.DEL.PROVIDER/index.js\"></script><link rel=\"stylesheet\" crossorigin href=\"http://IP.DEL.PROVIDER/index.css\"></head><body><div id=\"root\"></div></body></html>";

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
uint8_t turno = WHITE; // WHITE | BLACK

// Messaggi
#define mandaMess(id, msg) ws.text(players[(id)], (msg))
#define MSG_YOUR_TURN "MSG-È il tuo turno."
#define MSG_OPP_TURN "MSG-È il turno dell'avversario."
#define MSG_OPP_LEFT "MSG-L'avversario si è disconnesso :("
#define MSG_MOVING "MSG-Muovo la pedina..."
#define PLAY "PLAY"
#define FREEZE "FREEZE"
#define OPP_FOUND "OPP_FOUND"
#define OPP_LEFT "OPP_DISCONNECTED"

typedef struct {
  uint8_t column;
  uint8_t row;
} Square;
typedef struct {
  Square from, to;
  bool isPromotion = false;
} Move;

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

#define getOpp(id) ((id == players[WHITE]) ? BLACK : WHITE)
#define getIdx(id) ((id == players[WHITE]) ? WHITE : BLACK)
#define castUint8(x) (static_cast<uint8_t>(x))
void handleWebSocketMessage(uint32_t clientId, void *arg, uint8_t *data, size_t len) {

  AwsFrameInfo * info = (AwsFrameInfo*)arg; // faccio un cast dell'argomento in un AwsFrameInfo

  if (info->final && info->index == 0 && info->len == len) { // se il buffer del messaggio è pronto

    if (info->opcode == WS_TEXT) { // se l'opcode del messaggio è TEXT

      data[len] = 0; // aggiungo il terminatore di stringa in fondo al buffer
      Serial.printf("Client #%u ha inviato: %s\n", clientId, (char*)data); // printo il messaggio
      String msg = String((char*)data); // converto il buffer in una stringa

      if (msg.startsWith("MOVE") || msg.startsWith("PRMT")) {
        Move move;
        move.from = { castUint8(msg.charAt(5) - 'a') ,
                      castUint8(msg.charAt(6) - '1') };
        move.to  =  { castUint8(msg.charAt(8) - 'a') ,
                      castUint8(msg.charAt(9) - '1') };
        move.isPromotion = msg.startsWith("PRMT");

        ws.textAll(FREEZE); // Blocco i clients
        mandaMess(getOpp(clientId), msg.substring(11)); // Invio la mossa all'avversario
        ws.textAll(MSG_MOVING); // "Muovo la pedina..."
        turno = getOpp(clientId); // Cambio il turno

        muoviBraccio(move); // Muovo il braccio
      }
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

      if (players[WHITE] >= 0 && players[BLACK] >= 0) {
        ws.textAll(OPP_FOUND);
        mandaMess(WHITE, PLAY);
        mandaMess(BLACK, MSG_OPP_TURN);
        mandaMess(WHITE, MSG_YOUR_TURN);
      }

      Serial.printf("Connesso.\n");
      break;
    //////////////////////////////////////////////////////
    case WS_EVT_DISCONNECT: // Disconnessione di un client
      Serial.printf("Client #%u in disconnessione... ", client->id());

      // -- Rimozione del ruolo
      if (players[WHITE] == client->id()) players[WHITE] = -1;
      else if (players[BLACK] == client->id()) players[BLACK] = -1;
      else { Serial.printf("Connessione rifiutata.\n"); break; }
      // --

      ws.textAll(OPP_LEFT);
      ws.textAll(MSG_OPP_LEFT);

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
  Serial.onReceive([](){ concludiMovimento(); Serial.flush(); });
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

// Riprende la partita dopo il movimento della pedina
void concludiMovimento() {
  mandaMess(turno, MSG_YOUR_TURN); // invio a chi tocca
  mandaMess(!turno, MSG_OPP_TURN); // invio all'avversario
  mandaMess(turno, PLAY); // invio a chi tocca
}

// Funzione per muovere il braccio
// !!!! NON METTERE DEI DELAY !!!!
void muoviBraccio(Move mossa) {
  Serial.printf("%s: (%u,%u) -> (%u,%u)\n", mossa.isPromotion ? "Promozione" : "Mossa", mossa.from.column, mossa.from.row, mossa.to.column, mossa.to.row);
  
  ///////////////////////////////////////////////////////////
  ////////////////// MOVIMENTO DEL BRACCIO //////////////////
  ///////////////////////////////////////////////////////////
}