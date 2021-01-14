import React from 'react';
import Feed from './components/Feed';
import Navbar from './components/Navbar';
import Scrollable from './components/Scrollable';
import exampleNotes from './exampleNotes';

class App extends React.Component {

    constructor(props) {
        super(props)


        this.state = {
            searchText: '',
            notes: []
        }

        this.handleRefresh = this.handleRefresh.bind(this)
        this.handleAddNote = this.handleAddNote.bind(this)
    }

    async componentDidMount() {
        await this.handleRefresh()
    }

    async handleRefresh(x) {
        if (x && x.action) {
            switch (x.action) {
                case 'update':
                    this.setState({
                        notes: this.state.notes.map(note => note.id === x.id ? {
                            ...note,
                            content: x.content
                        } : note)
                    })
                    break;

                case 'update':
                    this.setState({
                        notes: this.state.notes.filter(note => note.id !== x.id)
                    })
                    break;
            }
        } else {
            try {
                const response = await fetch(`${window.location.href}notes`)
                if (response.ok)
                    this.setState({ notes: (await response.json()).map(note => ({ ...note, content: decodeURIComponent(note.content) })) })
                else
                    console.error(await response.text())
            } catch (error) {
                console.error(error)
            }
        }
    }

    async handleAddNote() {
        try {
            const response = await fetch(`${window.location.href}notes/add`)
            if (!response.ok)
                console.error(await response.text())
            await this.handleRefresh()
        } catch (error) {
            console.error(error)
        }
    }

    render() {
        return (
            <>
                <Scrollable>
                    <Feed notes={this.state.notes} onRefresh={this.handleRefresh} searchText={this.state.searchText} />
                </Scrollable>
                <Navbar onRefresh={this.handleRefresh} onAdd={this.handleAddNote} onSearchText={searchText => this.setState({ searchText: searchText })} />
            </>
        )
    }
}

export default App