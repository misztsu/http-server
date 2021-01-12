import { Link } from '@material-ui/core'
import Box from '@material-ui/core/Box'
import ButtonBase from '@material-ui/core/ButtonBase'
import Card from '@material-ui/core/Card'
import CardActions from '@material-ui/core/CardActions'
import CardContent from '@material-ui/core/CardContent'
import Checkbox from '@material-ui/core/Checkbox'
import Collapse from '@material-ui/core/Collapse'
import Divider from '@material-ui/core/Divider'
import IconButton from '@material-ui/core/IconButton'
import { withStyles } from '@material-ui/core/styles'
import Typography from '@material-ui/core/Typography'
import DeleteIcon from '@material-ui/icons/Delete'
import EditIcon from '@material-ui/icons/Edit'
import PaletteIcon from '@material-ui/icons/Palette'
import MDEditor from '@uiw/react-md-editor'
import React from 'react'
import ReactMarkdown from 'react-markdown'
import gfm from 'remark-gfm'

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
        borderLeftColor: theme.typography.body1.fontFamily,
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderTopRightRadius: '4px',
        borderBottomRightRadius: '4px'
    },
    code: {
        paddingLeft: '4px',
        paddingRight: '4px',
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderRadius: '4px',
        whiteSpace: 'pre'
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
            expanded: false,
            content: props.content
        }

        this.expand = this.expand.bind(this)
        this.delete = this.delete.bind(this)

    }

    expand() {
        this.setState({ expanded: !this.state.expanded })
    }

    delete() {
        this.props.onDelete(this.props.id)
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
                                    inlineCode: props => <Box className={classes.code} fontFamily="Monospace" fontSize="1.2rem" component="span">{props.children}</Box>,
                                    code: props => <Box className={classes.code} fontFamily="Monospace" fontSize="1.2rem" component="p">{props.value}</Box>,
                                    blockquote: props => <Box className={classes.blockquote} component="p">{props.children}</Box>,
                                    listItem: props => props.checked === null ? <li>{props.children}</li> : <li style={{ marginLeft: '-18px', listStyleType: 'none' }}><Checkbox style={{ padding: '0 4px 0 0' }} size="small" checked={Boolean(props.checked)} />{props.children}</li>,
                                    thematicBreak: Divider
                                }}
                                escapeHtml={false}
                            />
                        </Typography>
                    </CardContent>
                </ButtonBase>
                <CardActions disableSpacing>

                    <IconButton onClick={this.like} onClick={this.delete}>
                        <DeleteIcon fontSize="small" />
                    </IconButton>

                    <IconButton>
                        <PaletteIcon fontSize="small" />
                    </IconButton>

                    <IconButton onClick={this.expand}>
                        <EditIcon fontSize="small" />
                    </IconButton>

                </CardActions>
                <Collapse in={this.state.expanded} timeout="auto" >
                    <MDEditor
                        value={this.state.content}
                        height={240}
                        onChange={content => this.setState({ content: content })}
                        preview="edit"
                        className={classes.editor}
                    />
                </Collapse>
            </Card >
        )
    }
}

Note.defaultProps = {
    onDelete: (_) => { },
    content: ''
}

export default withStyles(useStyles)(Note)