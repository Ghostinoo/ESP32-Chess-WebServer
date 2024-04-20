import React, { useEffect, useState } from 'react';
import Board from './Board';
import { Chess, Square } from 'chess.js';

const App: React.FC = () => {
  const [message, _setMessage] = useState('Aspettando un avversario...');
  const [role, setRole] = useState<'WHITE' | 'BLACK' | null>(null);
  const [isThereOpponent, setIsThereOpponent] = useState(false);
  const [game, setGame] = useState(new Chess());
  const [canPlay, setCanPlay] = useState(false);

  const setMessage = (msg: string, bp: boolean = false) => { if (canPlay || bp) _setMessage(msg); };

  const updateGame = (newGame: Chess, payload?: {from: Square, to: Square}) => {
    setGame(newGame);
    const isMyTurn = (newGame.turn() === 'w' && role === 'WHITE' || newGame.turn() === 'b' && role === 'BLACK');
    if (newGame.isGameOver()) {
      if (newGame.isCheck() || game.inCheck())
        setMessage(!isMyTurn ? "L'hai messo in scacco!" : "Sei sotto scacco!");
      else if (newGame.isCheckmate())
        setMessage(!isMyTurn ? "Scacco matto! Hai vinto!" : "Hai perso :(");
      else if (newGame.isStalemate() || game.isDraw())
        setMessage('Sembra proprio un pareggio...');
      else setMessage('Partita finita!');
      }
    else setMessage('Sposto la pedina...');
    if (payload) {

    }
  };

  const onUpdateBoard = (fenString: string) => {
    const newGame = new Chess(fenString);
    updateGame(newGame);
  };

  useEffect(() => {
    const socket = new WebSocket('ws://192.168.1.202/ws');
    socket.onclose = e => console.log(`Connessione chiusa: ${e.reason}`);
    socket.onopen = () => console.log(`Connesso a ${socket.url}`);
    socket.onmessage = (e: MessageEvent) => {
      console.log(e.data);
      switch (e.data) {
        case 'WHITE':
        case 'BLACK':
          setRole(e.data);
          break;
        case 'FREEZE':
        case 'PLAY':
          setCanPlay(e.data === 'PLAY');
          setMessage(e.data === 'FREEZE'
            ? 'Sposto la pedina...'
            : !(game.turn() === 'w' && role === 'WHITE' || game.turn() === 'b' && role === 'BLACK')
              ? 'È il tuo turno!'
              : 'È il turno dell\'avversario!'
            , true
          );
          break;
        case 'OPP_DISCONNECTED':
          setIsThereOpponent(false);
          setMessage('Avversario disconnesso :(');
          break;
        case 'OPP_FOUND':
          setIsThereOpponent(true);
          game.reset();
          updateGame(new Chess(game.fen()));
          setMessage('Sposto la pedina...');
          break;
        default:
          onUpdateBoard(e.data);
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
