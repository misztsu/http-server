import { withStyles } from '@material-ui/core/styles'
import React from 'react'

const useStyles = (theme) => ({
    image: {
        maxWidth: '100%'
    },
    largeImage: {
        maxWidth: `calc(100% + ${theme.spacing(4)}px)`,
        marginLeft: -theme.spacing(2),
        marginRight: -theme.spacing(2)
    },
})

class Image extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            large: false
        }

        this.img = React.createRef()
        this.onLoad = this.onLoad.bind(this)
    }

    onLoad() {
        if (this.img.current.clientWidth < this.img.current.naturalWidth)
            this.setState({
                large: true
            })
    }

    componentDidMount() {
        this.img.current.addEventListener('load', this.onLoad)
    }

    componentWillUnmount() {
        this.img.current.removeEventListener('load', this.onLoad)
    }

    shouldComponentUpdate(nextProps, nextState) {
        return nextProps.src !== this.props.src || nextState.large !== this.state.large
    }

    render() {
        const { classes } = this.props
        return (
            <img
                ref={this.img}
                className={this.state.large ? classes.largeImage : classes.image}
                alt={this.props.alt}
                src={this.props.src}
            />
        )
    }
}

export default withStyles(useStyles)(Image)