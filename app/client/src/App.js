import { withSnackbar } from 'notistack';
import React from 'react';
import { Redirect, Route, Switch, withRouter } from 'react-router-dom';
import Feed from './components/Feed';
import Login from './components/Login';
import LoginHandler from './components/LoginHandler';
import Navbar from './components/Navbar';
import Scrollable from './components/Scrollable';
import SharedNoteHandler from './components/SharedNoteHandler';

class App extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            searchText: '',
            notes: JSON.parse(localStorage.getItem('notes')) || [],
            sharedNotes: JSON.parse(localStorage.getItem('sharedNotes')) || [],
            userId: localStorage.getItem('userId') || ''
        }

        this.handleRefresh = this.handleRefresh.bind(this)
        this.handleAddNote = this.handleAddNote.bind(this)
        this.handleAddSharedNote = this.handleAddSharedNote.bind(this)
        this.handleUser = this.handleUser.bind(this)
    }

    saveNotes() {
        localStorage.setItem('notes', JSON.stringify(this.state.notes))
    }

    saveSharedNotes() {
        localStorage.setItem('sharedNotes', JSON.stringify(this.state.sharedNotes))
    }

    async handleRefresh(command) {
        if (command && command.action) {
            switch (command.action) {
                case 'update':
                    this.setState({
                        notes: this.state.notes.map(note => note.noteId === command.noteId ? {
                            ...note,
                            content: command.content
                        } : note)
                    }, this.saveNotes)
                    break;

                case 'updateShared':
                    this.setState({
                        sharedNotes: this.state.sharedNotes.map(note => note.noteId === command.noteId && note.userId === command.userId ? {
                            ...note,
                            content: command.content
                        } : note)
                    }, this.saveSharedNotes)
                    break;

                case 'delete':
                    this.setState({
                        notes: this.state.notes.filter(note => note.noteId !== command.noteId),
                    }, this.saveNotes)
                    break;

                case 'deleteShared':
                    this.setState({
                        sharedNotes: this.state.sharedNotes.filter(note => !(note.noteId === command.noteId && note.userId === command.userId)),
                    }, this.saveSharedNotes)
                    break;

                case 'add':
                    this.setState({
                        notes: this.state.notes.concat([{ ...command, action: undefined }])
                    }, this.saveNotes)
                    break;

                case 'addShared':
                    if (this.state.sharedNotes.findIndex(note => note.noteId === command.noteId && note.userId === command.userId) === -1)
                        this.setState({
                            sharedNotes: this.state.sharedNotes.concat([{ ...command, action: undefined, shared: true }])
                        }, this.saveSharedNotes)
                    break;
            }
        } else if (this.state.userId) {
            try {
                const response = await fetch(`/users/${this.state.userId}/notes`, {
                    method: 'GET'
                })
                const body = await response.json()
                if (response.status == 200) {
                    this.setState({
                        notes: (await body).sort((a, b) => {
                            if (a.noteId < b.noteId) return -1
                            if (a.noteId > b.noteId) return 1
                            return 0
                        }).map(note => ({ ...note, content: note.content }))
                    }, this.saveNotes)
                } else if (response.status == 401) {
                    await this.handleUser('')
                    this.props.history.replace('/login')
                } else {
                    console.warn(body)
                    this.props.enqueueSnackbar(body.message, { variant: 'error' })
                }
                this.setState({
                    sharedNotes: (await Promise.all(this.state.sharedNotes.map(async note => {
                        try {
                            return {
                                ...(await (await fetch(`/users/${note.userId}/notes/${note.noteId}`, {
                                    method: 'GET'
                                })).json()),
                                shared: true
                            }
                        } catch (error) {
                            return null
                        }
                    }))).filter(note => note && note.userId && note.noteId)
                }, this.saveSharedNotes)
            } catch (error) {
                console.error(error)
                this.props.enqueueSnackbar('Error.', { variant: 'error' })
            }
        } else {
            this.props.history.replace('/login')
        }
    }

    async handleAddNote(content) {
        try {
            const response = await fetch(`/users/${this.state.userId}/notes/add`, {
                method: 'GET'
            })
            const body = await response.json()
            if (response.status == 201) {
                if (content && typeof content == 'string') {
                    const response2 = await fetch(`/users/${this.state.userId}/notes/${body.noteId}`, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({
                            content: content,
                            old: ''
                        })
                    })
                    const body2 = await response2.json()
                    if (response2.status == 200) {
                        await this.handleRefresh({ action: 'add', userId: this.state.userId, noteId: body2.noteId, content: content })
                    } else {
                        console.warn(body2)
                        this.props.enqueueSnackbar(body2.message, { variant: 'error' })
                    }
                } else {
                    await this.handleRefresh({ action: 'add', userId: this.state.userId, noteId: body.noteId, expanded: true })
                }
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
        }
    }

    async handleAddSharedNote(note) {
        try {
            const response = await fetch(`/users/${note.userId}/notes/${note.noteId}`, {
                method: 'GET'
            })
            const body = await response.json()
            if (response.status == 200) {
                await this.handleRefresh({ action: 'addShared', userId: body.userId, noteId: body.noteId, content: body.content })
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
        }
    }

    async handleUser(userId) {
        localStorage.removeItem('notes')
        localStorage.removeItem('sharedNotes')
        this.setState({ notes: [], sharedNotes: [] })
        if (userId === '') {
            localStorage.removeItem('userId')
            this.setState({ userId: '' }, () => this.handleRefresh())
        } else {
            localStorage.setItem('userId', userId)
            this.setState({ userId: userId, }, () => this.handleRefresh())
        }
    }

    render() {
        return (
            <Switch>
                <Route exact path="/login">
                    <Login onUser={this.handleUser} />
                </Route>
                <Route exact path="/shared/:userId/:noteId" render={(props) =>
                    <SharedNoteHandler onAddSharedNote={this.handleAddSharedNote} {...props} />
                }>
                </Route>
                <Route exact path="/">
                    <LoginHandler onRefresh={this.handleRefresh} />
                    <Scrollable>
                        <Feed notes={this.state.notes.concat(this.state.sharedNotes)} onAddNote={this.handleAddNote} onRefresh={this.handleRefresh} searchText={this.state.searchText} />
                    </Scrollable>
                    <Navbar userId={this.state.userId} onUser={this.handleUser} onRefresh={this.handleRefresh} onAddNote={this.handleAddNote} onAddSharedNote={this.handleAddSharedNote} onSearchText={searchText => this.setState({ searchText: searchText })} />
                </Route>
                <Redirect to="/" />
            </Switch>
        )
    }
}

export default withSnackbar(withRouter(App))