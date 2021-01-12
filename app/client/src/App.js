import React from 'react';
import Feed from './components/Feed';
import Navbar from './components/Navbar';
import Scrollable from './components/Scrollable';

class App extends React.Component {

    constructor(props) {
        super(props)

        this.state = {
            searchText: ''
        }
    }

    render() {
        return (
            <>
                <Scrollable>
                    <Feed searchText={this.state.searchText}/>
                </Scrollable>
                <Navbar onSearchText={searchText => this.setState({ searchText: searchText })}/>
            </>
        )
    }
}

export default App