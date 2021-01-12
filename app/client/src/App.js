import React from 'react';
import Feed from './components/Feed';
import Navbar from './components/Navbar';
import Scrollable from './components/Scrollable';
import exampleNotes from './exampleNotes';

class App extends React.Component {

    constructor(props) {
        super(props)

        const notes = JSON.parse(localStorage.getItem('notes')) || exampleNotes

        this.state = {
            searchText: '',
            notes: notes,
            maxId: notes.reduce((max, note) => (note.id > max ? note.id : max), notes[0].id)
        }

        this.handleRefresh = this.handleRefresh.bind(this)
        this.handleAddNote = this.handleAddNote.bind(this)
    }

    handleRefresh() {
        this.setState({ notes: JSON.parse(localStorage.getItem('notes')) })
    }

    handleAddNote() {
        localStorage.setItem('notes', JSON.stringify(
            JSON.parse(localStorage.getItem('notes')).concat([{
                id: this.state.maxId + 1,
                content: ''
            }])
        ))

        this.setState({
            notes: this.state.notes.concat([{
                id: this.state.maxId + 1,
                content: '',
                expanded: true
            }])
        })

        this.setState({ maxId: this.state.maxId + 1 })
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