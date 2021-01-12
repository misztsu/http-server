import { withStyles } from '@material-ui/core/styles'
import React from 'react'
import { Scrollbars } from 'react-custom-scrollbars'

const useStyles = (theme) => ({
    main: {
        height: '100%',
        display: 'flex',
        flexDirection: 'column'
    },
    thumb: {
        cursor: 'pointer',
        borderRadius: 'inherit',
        backgroundColor: theme.palette.text.secondary
    },
    toolbar: theme.mixins.toolbar
})

class Scrollable extends React.Component {

    constructor(props) {
        super(props)
        this.renderThumb = this.renderThumb.bind(this)
    }


    renderThumb({ style }) {
        return <div style={style} className={this.props.classes.thumb} />
    }

    render() {
        const { classes } = this.props
        return (
            <div className={classes.main}>
                <Scrollbars style={{ flexGrow: 1 }} autoHide renderThumbHorizontal={this.renderThumb} renderThumbVertical={this.renderThumb}>
                    {this.props.children}
                </Scrollbars>
                <div className={classes.toolbar} />
            </div>
        )
    }
}

export default withStyles(useStyles)(Scrollable)