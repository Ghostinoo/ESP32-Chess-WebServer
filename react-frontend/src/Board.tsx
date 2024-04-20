import React, { useState } from "react";
import { Chess, Square } from "chess.js";
import { Chessboard } from "react-chessboard";
import { PromotionPieceOption } from "react-chessboard/dist/chessboard/types";
import { useEventListener } from "usehooks-ts";

// https://react-chessboard.vercel.app/?path=/docs/example-chessboard--click-to-move

const boardSizeFactor = .8;
const computeWidth = () => window.innerWidth < window.innerHeight ? window.innerWidth * boardSizeFactor : window.innerHeight * boardSizeFactor;

const Board: React.FC<{
  role: "WHITE" | "BLACK" | null;
  isThereOpponent: boolean;
  game: Chess;
  setGame: (game: Chess, payload?: {from: Square, to: Square}) => void;
  canPlay: boolean;
}> = ({
  role,
  isThereOpponent,
  game,
  setGame,
  canPlay,
}) => {
  const [moveFrom, setMoveFrom] = useState<Square | "">("");
  const [moveTo, setMoveTo] = useState<Square | null>(null);
  const [showPromotionDialog, setShowPromotionDialog] = useState(false);
  const [rightClickedSquares, setRightClickedSquares] = useState({});
  // const [moveSquares, setMoveSquares] = useState({});
  const [optionSquares, setOptionSquares] = useState({});
  const [boardWidth, setBoardWidth] = useState(computeWidth());

  const isMyTurn = game.turn() === "w" && role === "WHITE" || game.turn() === "b" && role === "BLACK";

  function getMoveOptions(square: Square) {
    const moves = game.moves({
      square,
      verbose: true,
    });
    if (moves.length === 0) {
      setOptionSquares({});
      return false;
    }

    const newSquares: Record<string, any> = {};
    moves.map((move) => {
      newSquares[move.to] = {
        background:
          game.get(move.to) &&
          game.get(move.to).color !== game.get(square).color
            ? "radial-gradient(circle, rgba(0,0,0,.3) 60%, transparent 60%)"
            : "radial-gradient(circle, rgba(0,0,0,.2) 25%, transparent 25%)",
        borderRadius: "50%",
      };
      return move;
    });
    newSquares[square] = {
      background: "rgba(255, 255, 0, 0.4)",
    };
    setOptionSquares(newSquares);
    return true;
  }

  function onSquareClick(square: Square) {
    if (!isMyTurn || !isThereOpponent || !canPlay) return;
    setRightClickedSquares({});

    // from square
    if (!moveFrom) {
      const hasMoveOptions = getMoveOptions(square);
      if (hasMoveOptions) setMoveFrom(square);
      return;
    }

    // to square
    if (!moveTo) {
      // check if valid move before showing dialog
      const moves = game.moves({
        square: moveFrom as Square,
        verbose: true,
      });
      const foundMove = moves.find(
        (m) => m.from === moveFrom && m.to === square
      );
      // not a valid move
      if (!foundMove) {
        // check if clicked on new piece
        const hasMoveOptions = getMoveOptions(square);
        // if new piece, setMoveFrom, otherwise clear moveFrom
        setMoveFrom(hasMoveOptions ? square : "");
        return;
      }

      // valid move
      setMoveTo(square);

      // if promotion move
      if (
        (foundMove.color === "w" &&
          foundMove.piece === "p" &&
          square[1] === "8") ||
        (foundMove.color === "b" &&
          foundMove.piece === "p" &&
          square[1] === "1")
      ) {
        setShowPromotionDialog(true);
        return;
      }

      // is normal move
      const move = game.move({
        from: moveFrom,
        to: square,
        promotion: "q",
      });

      // if invalid, setMoveFrom and getMoveOptions
      if (move === null) {
        const hasMoveOptions = getMoveOptions(square);
        if (hasMoveOptions) setMoveFrom(square);
        return;
      }

      if (move !== null)
        setGame(new Chess(game.fen()), {
          from: move.from,
          to: move.to,
        });

      // setTimeout(makeRandomMove, 300);
      setMoveFrom("");
      setMoveTo(null);
      setOptionSquares({});
      return;
    }
  }

  function onPromotionPieceSelect(piece?: PromotionPieceOption) {
    // if no piece passed then user has cancelled dialog, don't make move and reset
    if (piece && moveTo) {
      game.move({
        from: moveFrom,
        to: moveTo,
        promotion: piece[1].toLowerCase() ?? "q",
      });
      setGame(new Chess(game.fen()));
      // setTimeout(makeRandomMove, 300);
    }

    setMoveFrom("");
    setMoveTo(null);
    setShowPromotionDialog(false);
    setOptionSquares({});
    return true;
  }

  useEventListener("resize", () => setBoardWidth(computeWidth()));

  return (
    <div className="board-wrapper">
      <Chessboard
        animationDuration={200}
        arePiecesDraggable={false}
        position={game.fen()}
        onSquareClick={onSquareClick}
        onPromotionPieceSelect={onPromotionPieceSelect}
        boardWidth={boardWidth}
        boardOrientation={role === "WHITE" ? "white" : "black"}
        customBoardStyle={{
          borderRadius: "10px",
          boxShadow: "0 0 2vw rgba(0, 0, 0, .7)",
        }}
        customSquareStyles={{
          // ...moveSquares,
          ...optionSquares,
          ...rightClickedSquares,
        }}
        promotionToSquare={moveTo}
        showPromotionDialog={showPromotionDialog}
      />
      {/* <button
        onClick={() => {
          if (!isThereOpponent) return;
          game.reset();
          setGame(new Chess(game.fen()));
          setMoveSquares({});
          setOptionSquares({});
          setRightClickedSquares({});
        }}
      >
        Reset
      </button> */}
    </div>
  );
};

export default Board;