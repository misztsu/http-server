import AppBar from '@material-ui/core/AppBar'
import Avatar from '@material-ui/core/Avatar'
import CssBaseline from '@material-ui/core/CssBaseline'
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
import Toolbar from '@material-ui/core/Toolbar'
import Zoom from '@material-ui/core/Zoom'
import AddIcon from '@material-ui/icons/Add'
import GitHubIcon from '@material-ui/icons/GitHub'
import MenuIcon from '@material-ui/icons/Menu'
import ReplayIcon from '@material-ui/icons/Replay'
import SearchIcon from '@material-ui/icons/Search'
import React from 'react'
import exampleNotes from '../exampleNotes'

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
            searchText: ''
        }

        this.handleSearchText = this.handleSearchText.bind(this)
        this.handleSearchTextSubmit = this.handleSearchTextSubmit.bind(this)
    }

    handleSearchText(e) {
        this.setState({ searchText: e.target.value })
        this.props.onSearchText(e.target.value)
    }

    handleSearchTextSubmit(e) {
        this.props.onSearchText(e.target.value)
        e.preventDefault()
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
                            <Fab color="primary" aria-label="add" className={classes.fabButton} onClick={this.props.onAdd}>
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
                                <ListItemText primary={'Kitty Notes'} />
                            </ListItem>
                        </List>
                        <Divider />
                        <List>
                            <ListItem button onClick={async () => {
                                try {
                                    const response = await fetch(`${window.location.href}notes/loadExamples`)
                                    if (response.ok)
                                        this.props.onRefresh()
                                    else
                                        console.error(await response.text())
                                } catch (error) {
                                    console.error(error)
                                }
                            }}>
                                <ListItemIcon><ReplayIcon /></ListItemIcon>
                                <ListItemText primary={'Load example notes'} />
                            </ListItem>
                            <ListItem button component="a" href="https://github.com/misztsu/http-server">
                                <ListItemIcon><GitHubIcon /></ListItemIcon>
                                <ListItemText primary={'Visit GitHub repository'} />
                            </ListItem>
                        </List>
                    </Drawer>
                </div>
            </>
        )
    }
}

Navbar.defaultProps = {
    onSearchText: (_) => { },
    onAdd: () => { },
    onRefresh: (_) => { }
}

export default withStyles(useStyles)(Navbar)