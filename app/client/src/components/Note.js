import { Link } from '@material-ui/core';
import Box from '@material-ui/core/Box';
import Button from '@material-ui/core/Button';
import ButtonBase from '@material-ui/core/ButtonBase';
import Card from '@material-ui/core/Card';
import CardActions from '@material-ui/core/CardActions';
import CardContent from '@material-ui/core/CardContent';
import Checkbox from '@material-ui/core/Checkbox';
import Collapse from '@material-ui/core/Collapse';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import Divider from '@material-ui/core/Divider';
import IconButton from '@material-ui/core/IconButton';
import { withStyles } from '@material-ui/core/styles';
import TextField from '@material-ui/core/TextField';
import Typography from '@material-ui/core/Typography';
import CloudIcon from '@material-ui/icons/Cloud';
import DeleteIcon from '@material-ui/icons/Delete';
import EditIcon from '@material-ui/icons/Edit';
import SaveIcon from '@material-ui/icons/Save';
import ShareIcon from '@material-ui/icons/Share';
import MDEditor from '@uiw/react-md-editor';
import React from 'react';
import { CopyToClipboard } from 'react-copy-to-clipboard';
import ReactMarkdown from 'react-markdown';
import gfm from 'remark-gfm';

const useStyles = (theme) => ({
    content: {
        paddingTop: theme.spacing(1),
        paddingBottom: theme.spacing(1)
    },
    buttonBase: {
        display: 'block'
    },
    blockquote: {
        paddingLeft: theme.spacing(1),
        paddingRight: '4px',
        borderLeftWidth: '4px',
        borderLeftStyle: 'solid',
        borderLeftColor: theme.palette.primary.main,
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderTopRightRadius: '4px',
        borderBottomRightRadius: '4px',
        whiteSpace: 'pre',
        display: 'block'
    },
    code: {
        paddingLeft: '4px',
        paddingRight: '4px',
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderRadius: '4px',
        whiteSpace: 'pre',
        display: 'block'
    },
    li: {
        marginLeft: '-18px',
        listStyleType: 'none'
    },
    checkbox: {
        padding: '0 4px 4px 0'
    },
    editor: {
        '--background-paper': theme.palette.background.paper,
        '--font-family': theme.typography.body1.fontFamily,
        '--font-size': theme.typography.body1.fontSize,
        '--color': theme.palette.text.primary,
        '--primary-color': theme.palette.primary.dark,
    }
})

class Note extends React.Component {
    constructor(props) {
        super(props)

        this.state = {
            expanded: props.expanded,
            content: props.content,
            key: 0,
            copySharedNoteUrl: false
        }

        this.expand = this.expand.bind(this)
        this.delete = this.delete.bind(this)

    }

    componentDidUpdate(prevProps) {
        if (this.props.content !== prevProps.content)
            this.setState({ content: this.props.content, key: this.state.key + 1 })
    }

    async expand() {
        if (this.state.expanded) {
            try {
                const response = await fetch(`/users/${this.props.userId}/notes/${this.props.noteId}/update`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        content: this.state.content
                    })
                })
                const body = await response.json()
                if (response.status == 200) {
                    this.props.onRefresh({
                        action: 'update',
                        noteId: this.props.noteId,
                        content: this.state.content
                    })
                } else {
                    console.warn(body)
                    this.props.enqueueSnackbar(body.message, { variant: 'error' })
                }
            } catch (error) {
                console.error(error)
                this.props.enqueueSnackbar('Error.', { variant: 'error' })
            }
        }
        this.setState({ expanded: !this.state.expanded })
    }

    async delete() {
        try {
            const response = await fetch(`/users/${this.props.userId}/notes/${this.props.noteId}/delete`, {
                method: 'GET'
            })
            const body = await response.json()
            if (response.status == 200) {
                this.props.onRefresh({
                    action: 'delete',
                    noteId: this.props.noteId
                })
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
            <Card>
                <ButtonBase component="div" className={classes.buttonBase}>
                    <CardContent className={classes.content}>
                        <Typography component="div" variant="body1">
                            <ReactMarkdown
                                plugins={[[gfm, { singleTilde: false }]]}
                                source={this.state.content}
                                renderers={{
                                    link: Link,
                                    div: props => <div>{props.children}</div>,
                                    strong: props => <Box fontWeight="fontWeightBold" component="span">{props.children}</Box>,
                                    emphasis: props => <Box fontStyle="italic" component="span">{props.children}</Box>,
                                    inlineCode: props => <Box className={classes.code} fontFamily="Monospace" component="span">{props.children}</Box>,
                                    code: props => <Box className={classes.code} fontFamily="Monospace" component="span">{props.value}</Box>,
                                    blockquote: props => <Box className={classes.blockquote} component="span">{props.children}</Box>,
                                    listItem: props => props.checked === null ? <li>{props.children}</li> : <li className={classes.li}><Checkbox color="primary" size="small" className={classes.checkbox} checked={Boolean(props.checked)} />{props.children}</li>,
                                    thematicBreak: Divider
                                }}
                                escapeHtml={false}
                            />
                        </Typography>
                    </CardContent>
                </ButtonBase>
                <CardActions disableSpacing>

                    <IconButton onClick={this.delete}>
                        <DeleteIcon />
                    </IconButton>

                    <IconButton onClick={() => this.setState({ copySharedNoteUrl: true })}>
                        <ShareIcon />
                    </IconButton>

                    <IconButton onClick={this.expand}>
                        {this.state.expanded ? <SaveIcon /> : <EditIcon />}
                    </IconButton>

                    {!this.props.shared ? null :
                        <IconButton fontSize="small" color="primary" onClick={() => this.setState({ copySharedNoteUrl: true })}>
                            <CloudIcon fontSize="small" />
                        </IconButton>
                    }

                </CardActions>
                <Collapse in={this.state.expanded} timeout="auto" >
                    <MDEditor
                        key={this.state.key}
                        value={this.state.content}
                        height={240}
                        onChange={content => this.setState({ content: content })}
                        preview="edit"
                        className={classes.editor}
                    />
                </Collapse>

                <Dialog onClose={() => this.setState({ copySharedNoteUrl: false })} open={this.state.copySharedNoteUrl}>
                    <DialogTitle>Share with others</DialogTitle>
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
                        <Button onClick={() => this.setState({ copySharedNoteUrl: false })}>Cancel</Button>
                        <CopyToClipboard text={`${window.location.origin}/shared/${this.props.userId}/${this.props.noteId}`}
                            onCopy={() => this.setState({ copySharedNoteUrl: false })}>
                            <Button color="primary">Copy</Button>
                        </CopyToClipboard>
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

export default withStyles(useStyles)(Note)