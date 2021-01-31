import AppBar from '@material-ui/core/AppBar'
import Avatar from '@material-ui/core/Avatar'
import Button from '@material-ui/core/Button'
import CssBaseline from '@material-ui/core/CssBaseline'
import Dialog from '@material-ui/core/Dialog'
import DialogActions from '@material-ui/core/DialogActions'
import DialogContent from '@material-ui/core/DialogContent'
import DialogContentText from '@material-ui/core/DialogContentText'
import DialogTitle from '@material-ui/core/DialogTitle'
import Divider from '@material-ui/core/Divider'
import Drawer from '@material-ui/core/Drawer'
import Fab from '@material-ui/core/Fab'
import IconButton from '@material-ui/core/IconButton'
import InputBase from '@material-ui/core/InputBase'
import List from '@material-ui/core/List'
import ListItem from '@material-ui/core/ListItem'
import ListItemAvatar from '@material-ui/core/ListItemAvatar'
import ListItemIcon from '@material-ui/core/ListItemIcon'
import ListItemText from '@material-ui/core/ListItemText'
import Paper from '@material-ui/core/Paper'
import { withStyles } from '@material-ui/core/styles'
import TextField from '@material-ui/core/TextField'
import Toolbar from '@material-ui/core/Toolbar'
import Zoom from '@material-ui/core/Zoom'
import AddIcon from '@material-ui/icons/Add'
import CloudDownloadIcon from '@material-ui/icons/CloudDownload'
import ExitToAppIcon from '@material-ui/icons/ExitToApp'
import GitHubIcon from '@material-ui/icons/GitHub'
import MenuIcon from '@material-ui/icons/Menu'
import ReplayIcon from '@material-ui/icons/Replay'
import SearchIcon from '@material-ui/icons/Search'
import { withSnackbar } from 'notistack'
import React from 'react'
import { withRouter } from "react-router-dom"

const useStyles = (theme) => ({
    list: {
        marginBottom: theme.spacing(2),
        width: 250
    },
    appBar: {
        top: 'auto',
        bottom: 0,
        alignIItems: 'baseline'
    },
    searchField: {
        padding: '4px 4px',
        [`${theme.breakpoints.up('xs')} and (orientation: landscape)`]: {
            padding: '2px 2px',
        },
        [theme.breakpoints.up('sm')]: {
            padding: '8px 8px',
        },
        display: 'flex',
        flexGrow: 1
    },
    menuButton: {
        marginRight: theme.spacing(1)
    },
    input: {
        marginLeft: theme.spacing(1),
        flex: 1,
    },
    fabButton: {
        position: 'absolute',
        zIndex: 1,
        top: -30,
        left: 0,
        right: 0,
        margin: '0 auto',
    },
    main: {
        color: theme.palette.getContrastText(theme.palette.primary.main),
        backgroundColor: theme.palette.primary.main,
    },
})


class Navbar extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            searchText: '',
            sharedNoteUrl: '',
            loadSharedNoteUrl: false
        }

        this.handleSearchText = this.handleSearchText.bind(this)
        this.handleSearchTextSubmit = this.handleSearchTextSubmit.bind(this)
        this.handleLogout = this.handleLogout.bind(this)
        this.handleLoadExamples = this.handleLoadExamples.bind(this)
    }

    handleSearchText(e) {
        this.setState({ searchText: e.target.value })
        this.props.onSearchText(e.target.value)
    }

    handleSearchTextSubmit(e) {
        this.props.onSearchText(e.target.value)
        e.preventDefault()
    }

    async handleLogout(event) {
        event.preventDefault()
        try {
            const response = await fetch(`/users/logout`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    userId: this.props.userId
                })
            })
            const body = await response.json()
            if (response.status == 200) {
                this.props.enqueueSnackbar(`User "${this.props.userId}" logged out.`)
                this.props.onUser('')
                this.props.history.replace('/login')
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
        }
    }

    async handleLoadExamples(event) {
        event.preventDefault()
        try {
            const response = await fetch(`/users/${this.props.userId}/notes/loadExamples`, {
                method: 'POST'
            })
            const body = await response.json()
            if (response.status == 200) {
                //this.props.enqueueSnackbar('Loaded example notes.')
                this.props.onRefresh()
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
            <>
                <CssBaseline />
                <AppBar position="fixed" color="default" className={classes.appBar}>
                    <Toolbar>
                        <IconButton edge="start" color="inherit" className={classes.menuButton} onClick={() => this.setState({ open: true })}>
                            <MenuIcon />
                        </IconButton>

                        <Zoom in={this.state.searchText.length === 0}>
                            <Fab color="primary" aria-label="add" className={classes.fabButton} onClick={this.props.onAddNote}>
                                <AddIcon />
                            </Fab >
                        </Zoom>

                        <Paper component="form" className={classes.searchField} onSubmit={this.handleSearchTextSubmit}>
                            <InputBase
                                className={classes.input}
                                placeholder="Search"
                                onChange={this.handleSearchText}
                                value={this.state.searchText}
                            />
                            <IconButton type="submit" size="small" className={classes.iconButton}>
                                <SearchIcon />
                            </IconButton>
                        </Paper>
                    </Toolbar>
                </AppBar>
                <div
                    onClick={() => this.setState({ open: false })}
                    onKeyDown={() => this.setState({ open: false })}
                >
                    <Drawer anchor={'bottom'} open={this.state.open}>
                        <List className={classes.list}>
                            <ListItem>
                                <ListItemAvatar>
                                    <Avatar className={classes.main}>^•ﻌ•^</Avatar>
                                </ListItemAvatar>
                                <ListItemText primary={'Kitty Notes'} secondary={this.props.userId} />
                            </ListItem>
                        </List>
                        <Divider />
                        <List>
                            <ListItem button onClick={this.handleLoadExamples}>
                                <ListItemIcon><ReplayIcon /></ListItemIcon>
                                <ListItemText primary={'Load example notes'} />
                            </ListItem>
                            <ListItem button onClick={() => this.setState({ loadSharedNoteUrl: true })}>
                                <ListItemIcon><CloudDownloadIcon /></ListItemIcon>
                                <ListItemText primary={'Load shared note'} />
                            </ListItem>
                            <ListItem button onClick={this.handleLogout}>
                                <ListItemIcon><ExitToAppIcon /></ListItemIcon>
                                <ListItemText primary={'Logout'} />
                            </ListItem>
                            <ListItem button component="a" href="https://github.com/misztsu/http-server">
                                <ListItemIcon><GitHubIcon /></ListItemIcon>
                                <ListItemText primary={'Visit GitHub repository'} />
                            </ListItem>
                        </List>
                    </Drawer>
                </div>

                <Dialog onClose={() => this.setState({ loadSharedNoteUrl: false })} open={this.state.loadSharedNoteUrl}>
                    <DialogTitle>Load shared note</DialogTitle>
                    <DialogContent>
                        <DialogContentText>
                            Anyone with this link can view, edit and delete this note. Be careful!
                        </DialogContentText>
                        <TextField
                            autoFocus
                            value={this.state.sharedNoteUrl}
                            onChange={e => this.setState({ sharedNoteUrl: e.target.value })}
                            margin="dense"
                            variant="outlined"
                            label="Url"
                            fullWidth
                            error={
                                !/^https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]+\/shared\/(\w+)\/([0-9a-f]+)\/?$/.test(this.state.sharedNoteUrl)
                                && this.state.sharedNoteUrl.length > 0
                            }
                            helperText={
                                (/^https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]+\/shared\/(\w+)\/([0-9a-f]+)\/?$/.test(this.state.sharedNoteUrl)
                                    || this.state.sharedNoteUrl.length == 0) ? ''
                                    : 'This is not valid shared note url.'
                            }
                        />
                    </DialogContent>
                    <DialogActions>
                        <Button onClick={() => this.setState({ loadSharedNoteUrl: false })}>Cancel</Button>
                        <Button onClick={() => {
                            const note = this.state.sharedNoteUrl.match(/\/(\w+)\/([0-9a-f]+)\/?$/)
                            if (note && note.length > 2)
                                this.props.onAddSharedNote({
                                    userId: note[1],
                                    noteId: note[2]
                                })
                            this.setState({ loadSharedNoteUrl: false })
                        }} color="primary">Load</Button>
                    </DialogActions>
                </Dialog>
            </>
        )
    }
}

Navbar.defaultProps = {
    onSearchText: (_) => { },
    onAddNote: () => { },
    onAddSharedNote: () => { },
    onRefresh: (_) => { },
    onUser: (_) => { }
}

export default withStyles(useStyles)(withRouter(withSnackbar(Navbar)))