
// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "Polizia Postale";
const char* password = "pecorino";

// Set web server port number to 8080
WiFiServer server(8080);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

char fenStr[32];

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println("Set-Cookie: pinco=\"pallo\"");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("POST /boh") >= 0) {
              Serial.println("RESETTATO");
            }

            if (header.indexOf("POST /dispatchMove") >= 0) {
              Serial.println();
            }
            
            // Display the HTML web pageclient.println("<!doctype html>");client.println("<!doctype html>");
            client.println("<html lang=\"en\">");
            client.println("  <head>");
            client.println("    <meta charset=\"UTF-8\" />");
            client.println("    <link rel=\"icon\" type=\"image/png\" href=\"https://i.ibb.co/vB1mKmF/logo-Pascal.png\" />");
            client.println("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />");
            client.println("    <title>ESP32 Chess WebServer</title>");
            client.println("    <script type=\"module\" crossorigin src=\"https://ghostinoo.github.io/ESP/index.js\"></script>");
            client.println("    <link rel=\"stylesheet\" type=\"text/css\" crossorigin href=\"https://ghostinoo.github.io/ESP/index.css\">");
            client.println("  </head>");
            client.println("  <body>");
            client.println("    <div id=\"root\"></div>");
            client.println("  </body>");
            client.println("</html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
   