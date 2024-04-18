import React, { useEffect, useState } from 'react';
import Board from './Board';

const App: React.FC = () => {
  const [message, setMessage] = useState('Partita in corso...');
  const [role, setRole] = useState<'WHITE' | 'BLACK' | null>(null);

  const onUpdateBoard = (_fenString: string) => {
    // da fare
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
        default:
          onUpdateBoard(e.data);
      }
    };
    return () => socket.close();
  }, []);

  return (
    <div className='app-wrapper'>
      <h1>{message}</h1>
      <Board setWinner={setMessage} role={role} />
    </div>
  );
};

export default App;
