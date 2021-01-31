import Button from '@material-ui/core/Button';
import ButtonBase from '@material-ui/core/ButtonBase';
import Card from '@material-ui/core/Card';
import CardActions from '@material-ui/core/CardActions';
import CardContent from '@material-ui/core/CardContent';
import CircularProgress from '@material-ui/core/CircularProgress';
import Collapse from '@material-ui/core/Collapse';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import Grid from '@material-ui/core/Grid';
import IconButton from '@material-ui/core/IconButton';
import Paper from '@material-ui/core/Paper';
import { withStyles } from '@material-ui/core/styles';
import TextField from '@material-ui/core/TextField';
import Typography from '@material-ui/core/Typography';
import CloudIcon from '@material-ui/icons/Cloud';
import DeleteIcon from '@material-ui/icons/Delete';
import EditIcon from '@material-ui/icons/Edit';
import SaveIcon from '@material-ui/icons/Save';
import ShareIcon from '@material-ui/icons/Share';
import MDEditor from '@uiw/react-md-editor';
import { withSnackbar } from 'notistack';
import React from 'react';
import { CopyToClipboard } from 'react-copy-to-clipboard';
import Markdown from './Markdown';
import CloudDownloadIcon from '@material-ui/icons/CloudDownload';

const useStyles = (theme) => ({
    buttonBase: {
        display: 'block'
    },
    editor: {
        '--background-paper': theme.palette.background.paper,
        '--font-family': theme.typography.body1.fontFamily,
        '--font-size': theme.typography.body1.fontSize,
        '--color': theme.palette.text.primary,
        '--primary-color': theme.palette.primary.dark,
    },
    fabProgress: {
        position: 'absolute',
        top: 0,
        left: 0,
        zIndex: 1,
    },
    wrapper: {
        margin: theme.spacing(0),
        position: 'relative',
    },
    paper: {
        padding: theme.spacing(2)
    },
    deleteIcon: {
        marginLeft: 'auto',
        marginRight: 'auto',
        marginTop: theme.spacing(1),
        display: 'block'
    }
})

class Note extends React.Component {
    constructor(props) {
        super(props)

        this.state = {
            expanded: props.expanded,
            content: props.content,
            oldContent: props.content,
            webContent: '',
            key: 0,
            shareDialog: false,
            sharedDialog: false,
            updateDialog: false,
            deleteDialog: false,
            saveAsNewDialog: false,
            updating: false,
            deleting: false,
            saveAsNew: false
        }

        this.expand = this.expand.bind(this)
        this.update = this.update.bind(this)
        this.delete = this.delete.bind(this)
    }

    componentDidUpdate(prevProps) {
        if (this.props.content !== prevProps.content) {
            this.setState({ content: this.props.content, oldContent: this.props.content, key: this.state.key + 1 })
        }
    }

    async expand() {
        if (this.state.expanded) {
            await this.update()
        }
        this.setState({ expanded: !this.state.expanded })
    }

    async update() {
        this.setState({ updating: true })
        try {
            const response = await fetch(`/users/${this.props.userId}/notes/${this.props.noteId}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    content: this.state.content,
                    old: this.state.oldContent
                })
            })
            const body = await response.json()
            if (response.status == 200) {
                this.props.onRefresh({
                    action: this.props.shared ? 'updateShared' : 'update',
                    noteId: this.props.noteId,
                    userId: this.props.userId,
                    content: this.state.content
                })
                this.setState({ updating: false })
            } else if (response.status == 409) {
                this.setState({
                    webContent: body.content,
                    updateDialog: true
                })
            } else if (response.status == 404) {
                this.setState({ saveAsNewDialog: true })
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
                this.setState({ updating: false })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
            this.setState({ updating: false })
        }
    }

    async saveAsNew() {
        if (this.state.saveAsNew) {
            this.props.onAddNote(this.state.content)
        }
        this.setState({ saveAsNewDialog: false })
        this.setState({ updating: false })
        this.props.onRefresh({
            action: this.props.shared ? 'deleteShared' : 'delete',
            noteId: this.props.noteId,
            userId: this.props.userId
        })
    }

    async delete() {
        this.setState({ deleting: true })
        try {
            const response = await fetch(`/users/${this.props.userId}/notes/${this.props.noteId}`, {
                method: 'DELETE',
                body: JSON.stringify({
                    old: this.state.oldContent
                })
            })
            const body = await response.json()
            if (response.status == 200) {
                this.setState({ deleting: false })
                this.props.onRefresh({
                    action: this.props.shared ? 'deleteShared' : 'delete',
                    noteId: this.props.noteId,
                    userId: this.props.userId
                })
            } else if (response.status == 409) {
                this.setState({
                    webContent: body.content,
                    deleteDialog: true
                })
            } else if (response.status == 404) {
                this.props.enqueueSnackbar('Note was already deleted.')
                this.setState({ deleting: false })
                this.props.onRefresh({
                    action: this.props.shared ? 'deleteShared' : 'delete',
                    noteId: this.props.noteId,
                    userId: this.props.userId
                })
            } else {
                console.warn(body)
                this.props.enqueueSnackbar(body.message, { variant: 'error' })
                this.setState({ deleting: false })
            }
        } catch (error) {
            console.error(error)
            this.props.enqueueSnackbar('Error.', { variant: 'error' })
            this.setState({ deleting: false })
        }
    }

    render() {
        const { classes } = this.props
        return (
            <Card>
                <ButtonBase component="div" className={classes.buttonBase}>
                    <CardContent className={classes.content}>
                        <Typography component="div" variant="body1">
                            <Markdown content={this.state.content} />
                        </Typography>
                    </CardContent>
                </ButtonBase>
                <CardActions disableSpacing>

                    {!this.props.shared ? null :
                        <IconButton fontSize="small" color="primary" onClick={() => this.setState({ sharedDialog: true })}>
                            <CloudIcon fontSize="small" />
                        </IconButton>
                    }

                    {!this.props.shared ? null :
                        <IconButton fontSize="small" onClick={() => this.props.onAddNote(this.state.content)}>
                            <CloudDownloadIcon fontSize="small" />
                        </IconButton>
                    }

                    <div className={classes.wrapper}>
                        <IconButton onClick={this.delete}>
                            <DeleteIcon />
                        </IconButton>
                        {!this.state.deleting ? null : <CircularProgress size={48} className={classes.fabProgress} />}
                    </div>

                    <IconButton onClick={() => this.setState({ shareDialog: true })}>
                        <ShareIcon />
                    </IconButton>

                    <div className={classes.wrapper}>
                        <IconButton onClick={this.expand}>
                            {this.state.expanded ? <SaveIcon /> : <EditIcon />}
                        </IconButton>
                        {!this.state.updating ? null : <CircularProgress size={48} className={classes.fabProgress} />}
                    </div>

                </CardActions>
                <Collapse in={this.state.expanded} timeout="auto" >
                    <MDEditor
                        key={this.state.key}
                        value={this.state.content}
                        height={240}
                        onChange={content => {
                            if (content.length > 90000) {
                                this.props.enqueueSnackbar('Note length exceeded 90000 characters, cropping to fit the maximum length.')
                                this.setState({ content: content.substr(0, 90000) })
                            } else
                                this.setState({ content: content })
                        }}
                        preview="edit"
                        className={classes.editor}
                    />
                </Collapse>

                <Dialog onClose={() => this.setState({ shareDialog: false })} open={this.state.shareDialog || this.state.sharedDialog}>
                    <DialogTitle>
                        {this.state.shareDialog ? 'Share with others' : 'Shared note'}
                    </DialogTitle>
                    <DialogContent>
                        <DialogContentText>
                            Anyone with this link can view, edit and delete this note. Be careful!
                        </DialogContentText>
                        <TextField
                            autoFocus
                            value={`${window.location.origin}/shared/${this.props.userId}/${this.props.noteId}`}
                            onChange={() => { }}
                            margin="dense"
                            variant="outlined"
                            label="Url"
                            fullWidth
                        />
                    </DialogContent>
                    <DialogActions>
                        <Button onClick={() => this.setState({ shareDialog: false, sharedDialog: false })}>Cancel</Button>
                        <CopyToClipboard text={`${window.location.origin}/shared/${this.props.userId}/${this.props.noteId}`}
                            onCopy={() => this.setState({ shareDialog: false, sharedDialog: false })}>
                            <Button color="primary">Copy</Button>
                        </CopyToClipboard>
                    </DialogActions>
                </Dialog>

                <Dialog disableBackdropClick disableEscapeKeyDown onClose={() => this.setState({ updateDialog: false })} open={this.state.updateDialog}>
                    <DialogTitle>
                        Someone edited this note
                    </DialogTitle>
                    <DialogContent>
                        <DialogContentText>
                            Oops! Someone edited this note.
                        </DialogContentText>
                        <Grid container spacing={1} justify="space-around">
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Your version</Typography>
                                    <Markdown content={this.state.content} />
                                </Paper>
                            </Grid>
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Web version</Typography>
                                    <Markdown content={this.state.webContent} />
                                </Paper>
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                updateDialog: false,
                                oldContent: this.state.webContent,
                                webContent: ''
                            }, () => {
                                this.update()
                            })
                        }}>
                            Keep your version
                        </Button>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                updateDialog: false,
                                oldContent: this.state.webContent,
                                content: this.state.webContent,
                                key: this.state.key + 1,
                                webContent: '',
                            }, () => {
                                this.update()
                            })
                        }}>
                            Discard your version
                        </Button>
                    </DialogActions>
                </Dialog>

                <Dialog disableBackdropClick disableEscapeKeyDown onClose={() => this.setState({ deleteDialog: false })} open={this.state.deleteDialog}>
                    <DialogTitle>
                        Someone edited this note
                    </DialogTitle>
                    <DialogContent>
                        <DialogContentText>
                            Oops! Someone edited this note.
                        </DialogContentText>
                        <Grid container spacing={1} justify="space-around">
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Your version</Typography>
                                    <Markdown content={this.state.content} />
                                </Paper>
                            </Grid>
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Web version</Typography>
                                    <Markdown content={this.state.webContent} />
                                </Paper>
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                deleteDialog: false,
                                oldContent: this.state.webContent,
                                webContent: ''
                            }, () => {
                                this.delete()
                            })
                        }}>
                            Delete anyway
                        </Button>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                deleteDialog: false,
                                oldContent: this.state.webContent,
                                content: this.state.webContent,
                                key: this.state.key + 1,
                                webContent: '',
                                deleting: false
                            })
                        }}>
                            Don't delete
                        </Button>
                    </DialogActions>
                </Dialog>

                <Dialog disableBackdropClick disableEscapeKeyDown onClose={() => this.setState({ saveAsNewDialog: false })} open={this.state.saveAsNewDialog}>
                    <DialogTitle>
                        Someone deleted this note
                    </DialogTitle>
                    <DialogContent>
                        <DialogContentText>
                            Oops! Someone deleted this note.
                        </DialogContentText>
                        <Grid container spacing={1} justify="space-around">
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Your version</Typography>
                                    <Markdown content={this.state.content} />
                                </Paper>
                            </Grid>
                            <Grid item xs>
                                <Paper variant="outlined" className={classes.paper}>
                                    <Typography variant="button" component="div">Web version</Typography>
                                    <DeleteIcon className={classes.deleteIcon} fontSize="large" color="disabled" />
                                </Paper>
                            </Grid>
                        </Grid>
                    </DialogContent>
                    <DialogActions>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                saveAsNewDialog: false,
                                saveAsNew: false
                            }, () => {
                                this.saveAsNew()
                            })
                        }}>
                            Discard note
                        </Button>
                        <Button color="primary" onClick={() => {
                            this.setState({
                                saveAsNewDialog: false,
                                saveAsNew: true
                            }, () => {
                                this.saveAsNew()
                            })
                        }}>
                            Save as new
                        </Button>
                    </DialogActions>
                </Dialog>

            </Card >
        )
    }
}

Note.defaultProps = {
    onRefresh: (_) => { },
    content: '',
    expanded: false
}

export default withStyles(useStyles)(withSnackbar(Note))