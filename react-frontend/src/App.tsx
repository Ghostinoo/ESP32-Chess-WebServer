import React, { useEffect, useRef, useState } from 'react';
import Board from './Board';
import { Chess, Square } from 'chess.js';

interface MovePayload {
  from: Square;
  to: Square;
}

const App: React.FC = () => {
  const socketRef = useRef<WebSocket | null>(null);
  const [message, setMessage] = useState('Aspettando un avversario...');
  const [role, setRole] = useState<'WHITE' | 'BLACK'>('WHITE');
  const [isThereOpponent, setIsThereOpponent] = useState(false);
  const [game, setGame] = useState(new Chess());
  const [canPlay, setCanPlay] = useState(false);

  const updateGame = (newGame: Chess, payload?: MovePayload, isPromotion: boolean = false) => {
    setGame(newGame);
    const isMyTurn = (newGame.turn() === 'w' && role === 'WHITE' || newGame.turn() === 'b' && role === 'BLACK');
    if (payload && socketRef.current)
      if (socketRef.current.OPEN) {
        socketRef.current.send(`${isPromotion ? 'PRMT' : 'MOVE'}_${payload.from}_${payload.to}_${newGame.fen()}`);
      }
    if (newGame.isGameOver()) {
      socketRef.current?.close();
      if (newGame.isCheckmate())
        setMessage(!isMyTurn ? "Scacco matto! Hai vinto!" : "Hai perso :(");
      else if (newGame.isStalemate() || game.isDraw())
        setMessage('Sembra proprio un pareggio...');
      else setMessage('Partita finita!');
    }
  };

  const onUpdateBoard = (fenString: string) => {
    const newGame = new Chess(fenString);
    updateGame(newGame);
  };

  useEffect(() => {
    const socket = new WebSocket('ws://IP.DELLA.ESP32/ws');
    socketRef.current = socket;
    socket.onclose = e => console.warn(`Connessione chiusa: ${e.reason}`);
    socket.onopen = () => console.warn(`Connesso a ${socket.url}`);
    socket.onmessage = (e: MessageEvent) => {
      if (!(typeof e.data === 'string')) return;
      console.log(e.data);
      switch (e.data) {
        case 'WHITE':
        case 'BLACK':
          setRole(e.data);
          break;
        case 'FREEZE':
        case 'PLAY':
          setCanPlay(e.data === 'PLAY');
          break;
        case 'OPP_DISCONNECTED':
          setIsThereOpponent(false);
          break;
        case 'OPP_FOUND':
          setIsThereOpponent(true);
          game.reset();
          updateGame(new Chess(game.fen()));
          break;
        default:
          if (e.data.startsWith('MSG-')) setMessage(e.data.slice(4));
          else onUpdateBoard(e.data);
      }
    };
    return () => socket.close();
  }, []);

  return (
    <div className='app-wrapper'>
      <h1>{message}</h1>
      <Board
        role={role}
        isThereOpponent={isThereOpponent}
        game={game}
        setGame={updateGame}
        canPlay={canPlay}
      />
    </div>
  );
};

export default App;
