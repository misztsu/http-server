import Container from '@material-ui/core/Container'
import Grid from '@material-ui/core/Grid'
import { withStyles } from '@material-ui/core/styles'
import React from 'react'
import Note from './Note'


const useStyles = (theme) => ({
    paper: {
        marginTop: theme.spacing(8),
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
    },
    form: {
        width: '100%',
        marginTop: theme.spacing(1),
    },
    submit: {
        margin: theme.spacing(2, 0, 2),
    },
    container: {
        paddingTop: theme.spacing(4),
        paddingBottom: theme.spacing(4),
    },
    hidden: {
        display: 'none'
    }
})

class Feed extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            notes: JSON.parse(localStorage.getItem('notes'))
        }

        this.search = this.search.bind(this)
        this.handleNoteDelete = this.handleNoteDelete.bind(this)
    }

    search() {
        return this.state.notes.map(note => ({
            ...note,
            visible: this.props.searchText === '' || this.props.searchText.toLowerCase().split(/\s+/).every(el => note.content.toLowerCase().indexOf(el) !== -1)
        }))
    }

    handleNoteDelete(id) {
        this.setState({ notes: this.state.notes.filter(note => note.id != id) })
    }

    render() {
        const { classes } = this.props

        return (
            <Container maxWidth="sm" className={classes.container} >
                <Grid container spacing={4} wrap="nowrap" direction="column-reverse">
                    {
                        this.search().map(
                            (note, id) => (
                                <Grid item key={id} className={note.visible !== false ? null : this.props.classes.hidden}>
                                    <Note onDelete={this.handleNoteDelete} {...note} />
                                </Grid>
                            )
                        )
                    }
                </Grid>
            </Container >
        )
    }
}

Feed.defaultProps = {
    searchText: ''
}

export default withStyles(useStyles)(Feed)