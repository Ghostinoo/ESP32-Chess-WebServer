# FrontEnd Docs
TODO

# ESP Docs
### Connection to HTML page

To display (and then render on your client) the html code contained in the constant `htmlPage`, you must connect to the ESP address (printed on the serial monitor once you are connected to WiFi): `http://ip.of.the.esp/`

The `GET` request will receive a response with a `200` (OK) status and the body of that request will contain the content of the `htmlPage` const.

### Connection to the WebSocket server
> [!WARNING]
> The WebSocket handler is configured to be able to accept a maximum of two client connections. If you want to modify this option, you'll have to modify the value of the argument in the `ws.cleanupClients` method, positioned at the last row of the `loop` function.

To connec to to the WS endpoint, you need to connect to `ws://ip.of.the.esp/ws`.

### Functioning of WebSocket connection
Using this protocol, connected clients are constantly connected via a data stream to the server (ESP).

Each client can send a message to the server (not directly to other clients).
The server can do a `broadcast`, using the `ws.textAll` (or better `ws.printfAll`) method to reach all connected clients.
Otherwise, a message can be addressed to a specific client by specifying the client`s ID.

Every communication in WebSocket has an `OPCODE` that defines the type of content. In our case, all messages that are not of type `TEXT` will be ignored, since the data transmitted between devices are [`FEN Strings`](https://www.chess.com/terms/fen-chess) (which describes the checkerboard).
