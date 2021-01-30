import { Link } from '@material-ui/core';
import Box from '@material-ui/core/Box';
import Checkbox from '@material-ui/core/Checkbox';
import Divider from '@material-ui/core/Divider';
import { withStyles } from '@material-ui/core/styles';
import React from 'react';
import ReactMarkdown from 'react-markdown';
import gfm from 'remark-gfm';


const useStyles = (theme) => ({
    blockquote: {
        paddingLeft: theme.spacing(1),
        paddingRight: '4px',
        borderLeftWidth: '4px',
        borderLeftStyle: 'solid',
        borderLeftColor: theme.palette.primary.main,
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderTopRightRadius: '4px',
        borderBottomRightRadius: '4px',
        whiteSpace: 'pre-wrap',
        display: 'block'
    },
    code: {
        paddingLeft: '4px',
        paddingRight: '4px',
        backgroundColor: 'rgba(0, 0, 0, 0.16)',
        borderRadius: '4px',
        whiteSpace: 'pre-wrap',
        display: 'block'
    },
    li: {
        marginLeft: '-18px',
        listStyleType: 'none'
    },
    checkbox: {
        padding: '0 4px 4px 0'
    }
})

class Markdown extends React.Component {

    render() {
        const { classes } = this.props
        return (
            <ReactMarkdown
                plugins={[[gfm, { singleTilde: false }]]}
                source={this.props.content}
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
        )
    }
}

Markdown.defaultProps = {
    content: ''
}

export default withStyles(useStyles)(Markdown)