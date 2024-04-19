import React, { useEffect, useState } from 'react';
import Board from './Board';
import { Chess } from 'chess.js';

const App: React.FC = () => {
  const [message, setMessage] = useState('Aspettando un avversario...');
  const [role, setRole] = useState<'WHITE' | 'BLACK' | null>(null);
  const [isThereOpponent, setIsThereOpponent] = useState(false);
  const [game, setGame] = useState(new Chess());

  const onUpdateBoard = (_fenString: string) => {

  };

  useEffect(() => {
    const socket = new WebSocket('ws://192.168.1.202/ws');
    socket.onopen = () => {
      console.log('Connesso');
      socket.send('check');
    };
    socket.onmessage = (e: MessageEvent) => {
      console.log(e.data);
      switch (e.data) {
        case 'WHITE':
        case 'BLACK':
          setRole(e.data);
          break;
        case 'OPP_DISCONNECTED':
          setIsThereOpponent(false);
          setMessage('Avversario disconnesso :(');
          break;
        case 'OPP_FOUND':
          setIsThereOpponent(true);
          game.reset();
          setGame(new Chess(game.fen()));
          setMessage('Partita in corso...');
          break;
        default:
          onUpdateBoard(e.data);
      }
    };
    return () => socket.close();
  }, []);

  useEffect(() => {
  //   const isMyTurn = !(game.turn() === 'w' && role === 'WHITE' || game.turn() === 'b' && role === 'BLACK');
  //   if (game.isGameOver()) {
  //     // socketRef.current?.send('GAMEOVER');
  //     if (game.isCheck() || game.inCheck())
  //       setMessage(isMyTurn ? "L'hai messo in scacco!" : "Sei sotto scacco!");
  //     else if (game.isCheckmate())
  //       setMessage(isMyTurn ? "Scacco matto! Hai vinto!" : "Hai perso :(");
  //     else if (game.isStalemate() || game.isDraw())
  //       setMessage('Sembra proprio un pareggio...');
  //     else setMessage('Partita finita!');
  //     }
  //   else setMessage(isMyTurn ? 'È il tuo turno!' : 'È il turno dell\'avversario!');
  }, [game]);

  return (
    <div className='app-wrapper'>
      <h1>{message}</h1>
      <Board
        role={role}
        isThereOpponent={isThereOpponent}
        game={game}
        setGame={setGame}
      />
    </div>
  );
};

export default App;
