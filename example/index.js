import { AppRegistry } from 'react-native';
import App, { WindowListViewer } from './src/App';
import { name as appName } from './app.json';

AppRegistry.registerComponent(appName, () => App);
AppRegistry.registerComponent('WindowListViewer', () => WindowListViewer);
