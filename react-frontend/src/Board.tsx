import React, { useRef, useState } from 'react';
import { Chess } from 'chess.js';
import { Chessboard } from 'react-chessboard';

const Board: React.FC<{
  setWinner: (w: string) => void;
}> = ({
  setWinner,
}) => {
  const gameRef = useRef(new Chess());
  const game = gameRef.current;
  const [positions, setPos] = useState(game.fen());
  
  const makeMove = (move: string | {
    from: string;
    to: string;
    promotion?: string;
  }) => {
    const res = game.move(move);
    setPos(game.fen());
    if (game.isGameOver()) {
      setWinner(game.turn() === 'w' ? 'Hai perso!' : 'Hai vinto!');
    }
    return res;
  };

  const makeRandomMove = () => {
    const possibleMoves = game.moves();
    if (game.isGameOver() || possibleMoves.length === 0) {
      return;
    }
    const randomIndex = Math.floor(Math.random() * possibleMoves.length);
    makeMove(possibleMoves[randomIndex]);
  };

  const reset = () => {
    setWinner('Partita in corso...');
    game.reset();
    setPos(game.fen());
  };

  const onDrop = (sourceSquare: string, targetSquare: string) => {
    const move = makeMove({
      from: sourceSquare,
      to: targetSquare,
      promotion: 'q',
    });
    if (move === null) return false;
    setTimeout(makeRandomMove, 200);
    return true;
  };

  return (
    <>
      <div className="board-wrapper">
        <Chessboard
          position={positions}
          onPieceDrop={onDrop}
          customDarkSquareStyle={{ background: 'linear-gradient(110deg, #2c2424 30%, #3f3636)' }}
          customLightSquareStyle={{ backgroundColor: '#f0e2d9' }}
        />
      </div>
      <button onClick={reset}>Reset</button>
    </>
  );
};

export default Board;