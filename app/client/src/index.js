import CssBaseline from '@material-ui/core/CssBaseline';
import { ThemeProvider } from '@material-ui/core/styles';
import { SnackbarProvider } from 'notistack';
import React from 'react';
import { CookiesProvider } from 'react-cookie';
import ReactDOM from 'react-dom';
import { BrowserRouter } from 'react-router-dom';
import App from './App';
import './index.css';
import theme from './theme';

ReactDOM.render(
    <ThemeProvider theme={theme}>
        <CookiesProvider>
            <BrowserRouter>
                <SnackbarProvider maxSnack={3}>
                    <CssBaseline />
                    <App />
                </SnackbarProvider>
            </BrowserRouter>
        </CookiesProvider>
    </ThemeProvider>,
    document.querySelector('#root'),
);
