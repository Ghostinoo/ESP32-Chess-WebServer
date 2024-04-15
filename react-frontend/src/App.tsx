import React, { useState } from 'react';
import Board from './Board';


const App: React.FC = () => {
  const [message, setMessage] = useState('Partita in corso...');

  return (
    <div className='app-wrapper'>
      <h1>{message}</h1>
      <Board setWinner={setMessage} />
    </div>
  );
};

export default App;
