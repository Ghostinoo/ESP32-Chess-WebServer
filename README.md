# Documentazione FrontEnd
TODO

# Documentazione ESP
### Connessione alla pagina HTML
> [!NOTE]
> La pagina servita NON è statica, è frutto di una richiesta http all'indirizzo `0.0.0.0/`

Per visualizzare (e quindi renderizzare sul proprio client) il codice html contenuto nella costante `htmlPage`, è necessario collegarsi all'indirizzo della ESP (stampato sul monitor seriale una volta connessi al WiFi), con un URI vuoto: `http://ip.della.esp/`

La richiesta `GET` riceverà appunto una risposta con status `200` (OK) e corpo contenente il contenuto della sopracitata costante.

### Collegamento al server WebSocket
> [!WARNING]
> L'handler WebSocket è configurato per accettare un massimo di 2 client connessi. Qualora si voglia modificare l'opzione, è necessario cambiare il valore dell'argomento del metodo `ws.cleanupClients`, posizionato all'ultima riga della funzione `loop`.

Per il collegamento all'endpoint, bisogna collegarsi all'indirizzo `ws://ip.della.esp/ws`.

### Funzionamento della connessione WebSocket
Utilizzando questo protocollo, i client collegati sono costantemente connessi tramite uno stream di dati al server (ESP).

Ogni client può mandare un messaggio al server (non direttamente ad altri client).
Il server può fare un `broadcast`, utilizzando il metodo `ws.textAll` (o meglio `ws.printfAll`) per raggiungere tutti i client collegati.
Altrimenti, è possibile indirizzare un messaggio ad uno specifico client, specificando l'ID dello stesso.

Ogni comunicazione in WebSocket ha un `OPCODE` che definisce la tipologia del contenuto. Nel nostro caso tutti i messaggi che non sono di tipo `TEXT` verranno ignorati, siccome i dati trasmessi tra i dispositivi sono delle `FEN Strings` (che descrivono la scacchiera).