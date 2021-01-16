import React from 'react';
import { withCookies } from 'react-cookie';
import { Redirect } from 'react-router-dom';

class LogoutHandler extends React.Component {

    render() {
        if (this.props.cookies.get('token'))
            return <Redirect to="/" />
        else
            return <Redirect to="/login" />
    }
}

export default withCookies(LogoutHandler)