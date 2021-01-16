import React from 'react'
import { Redirect } from 'react-router-dom'

class SharedNoteHandler extends React.Component {
    render() {
        this.props.onAddSharedNote({
            userId: this.props.match.params.userId,
            noteId: this.props.match.params.noteId
        })
        return <Redirect to="/" />
    }
}

export default SharedNoteHandler