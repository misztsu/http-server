import Avatar from '@material-ui/core/Avatar'
import Button from '@material-ui/core/Button'
import Container from '@material-ui/core/Container'
import Grid from '@material-ui/core/Grid'
import { withStyles } from '@material-ui/core/styles'
import TextField from '@material-ui/core/TextField'
import Typography from '@material-ui/core/Typography'
import LockOutlinedIcon from '@material-ui/icons/LockOutlined'
import CryptoJS from 'crypto-js'
import { withSnackbar } from 'notistack'
import React from 'react'
import { withRouter } from "react-router-dom"


const useStyles = (theme) => ({
    column: {
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
    },
    avatar: {
        marginTop: theme.spacing(10),
        backgroundColor: theme.palette.primary.main
    },
    form: {
        width: '100%',
        marginTop: theme.spacing(1),
    },
    submit: {
        margin: theme.spacing(2, 0, 2),
    },
    or: {
        margin: theme.spacing(1, 0, 3),
    },
    warning: {
        margin: theme.spacing(0, 0, 0)
    }
})

class Login extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            loginUsername: '',
            loginPassword: '',
            registerUsername: '',
            registerPassword: '',
            repeatPassword: ''
        }

        this.handleLogin = this.handleLogin.bind(this)
        this.handleRegister = this.handleRegister.bind(this)
    }

    async handleLogin(event) {
        event.preventDefault()
        if (!/^\w{1,20}$/.test(this.state.loginUsername))
            return
        try {
            const response = await fetch(`/users/login`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    userId: this.state.loginUsername,
                    hash: CryptoJS.SHA256(this.state.loginPassword).toString(CryptoJS.enc.Base64)
                })
            })
            const body = await response.json()
            if (response.status == 200) {
                this.props.onUser(body.userId)
                this.props.history.replace('/')
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
        }
    }

    async handleRegister(event) {
        event.preventDefault()
        if (!/^\w{1,20}$/.test(this.state.registerUsername))
            return
        try {
            const response = await fetch(`/users/add`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    userId: this.state.registerUsername,
                    hash: CryptoJS.SHA256(this.state.registerPassword).toString(CryptoJS.enc.Base64)
                })
            })
            const body = await response.json()
            if (response.status == 201) {
                this.props.onUser(body.userId)
                this.props.history.replace('/')
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
        }
    }

    render() {
        const { classes } = this.props
        return (
            <Container component="main" maxWidth="md">
                <Grid
                    container
                    direction="row"
                    justify="space-evenly"
                    alignItems="flex-start"
                    spacing={4}
                >
                    <Grid item xs={12}>
                        <div className={classes.column}>
                            <Avatar className={classes.avatar}>
                                <LockOutlinedIcon />
                            </Avatar>
                        </div>
                    </Grid>
                    <Grid item xs={12} sm={6}>
                        <div className={classes.column}>
                            <Typography component="h1" variant="h4">
                                Log in
                            </Typography>
                            <form className={classes.form} onSubmit={this.handleLogin} noValidate>
                                <TextField
                                    value={this.state.loginUsername}
                                    onChange={e => this.setState({ loginUsername: e.target.value })}
                                    variant="outlined"
                                    margin="normal"
                                    required
                                    fullWidth
                                    label="Username"
                                    name="username"
                                    error={!/^\w{0,20}$/.test(this.state.loginUsername)}
                                    helperText={
                                        /^\w*$/.test(this.state.loginUsername) ? (
                                            /^\w{0,20}$/.test(this.state.loginUsername) ? ''
                                                : 'Username must have less than 21 characters.'
                                        )
                                            : 'Username must not contain whitespace and special characters.'
                                    }
                                    autoFocus
                                />
                                <TextField
                                    value={this.state.loginPassword}
                                    onChange={e => this.setState({ loginPassword: e.target.value })}
                                    variant="outlined"
                                    margin="normal"
                                    required
                                    fullWidth
                                    name="password"
                                    label="Password"
                                    type="password"
                                />
                                <Button
                                    type="submit"
                                    fullWidth
                                    variant="contained"
                                    color="primary"
                                    className={classes.submit}
                                >
                                    Login
                                </Button>
                            </form>
                        </div>
                    </Grid>
                    <Grid item xs={12} sm={6}>
                        <div className={classes.column}>
                            <Typography component="h1" variant="h4">
                                ... or create account
                            </Typography>
                            <form className={classes.form} onSubmit={this.handleRegister} noValidate>
                                <TextField
                                    value={this.state.registerUsername}
                                    onChange={e => this.setState({ registerUsername: e.target.value })}
                                    variant="outlined"
                                    margin="normal"
                                    required
                                    fullWidth
                                    label="Username"
                                    name="username"
                                    error={!/^\w{0,20}$/.test(this.state.registerUsername)}
                                    helperText={
                                        /^\w*$/.test(this.state.registerUsername) ? (
                                            /^\w{0,20}$/.test(this.state.registerUsername) ? ''
                                                : 'Username must have less than 21 characters.'
                                        )
                                            : 'Username must not contain whitespace and special characters.'
                                    }
                                />
                                <TextField
                                    value={this.state.registerPassword}
                                    onChange={e => this.setState({ registerPassword: e.target.value })}
                                    variant="outlined"
                                    margin="normal"
                                    required
                                    fullWidth
                                    name="password"
                                    label="Password"
                                    type="password"
                                />
                                <TextField
                                    value={this.state.repeatPassword}
                                    onChange={e => this.setState({ repeatPassword: e.target.value })}
                                    variant="outlined"
                                    margin="normal"
                                    required
                                    fullWidth
                                    name="repeatPassword"
                                    label="Repeat password"
                                    type="password"
                                    error={this.state.registerPassword !== this.state.repeatPassword && this.state.repeatPassword.length > 0}
                                    helperText=
                                    {
                                        (this.state.registerPassword === this.state.repeatPassword || this.state.repeatPassword.length == 0) ? ''
                                            : 'Passwords does not match.'
                                    }
                                />
                                <Button
                                    type="submit"
                                    fullWidth
                                    variant="contained"
                                    className={classes.submit}
                                >
                                    Register
                                </Button>
                            </form>
                        </div>
                    </Grid>
                </Grid>
            </Container>
        )
    }
}

Login.defaultProps = {
    onUser: (_) => { }
}

export default withStyles(useStyles)(withRouter(withSnackbar(Login)))