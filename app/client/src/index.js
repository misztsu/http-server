import CssBaseline from '@material-ui/core/CssBaseline';
import { ThemeProvider } from '@material-ui/core/styles';
import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import './index.css';
import theme from './theme';

localStorage.setItem('notes', JSON.stringify([
    {
        id: 1,
        content: 'a'
    },
    {
        id: 2,
        content: 'b'
    },
    {
        id: 3,
        content: 'c'
    }
]))

ReactDOM.render(
    <ThemeProvider theme={theme}>
        <CssBaseline />
        <App />
    </ThemeProvider>,
    document.querySelector('#root'),
);
