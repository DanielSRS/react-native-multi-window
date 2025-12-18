import { AppRegistry } from 'react-native';
import App, { Second } from './src/App';
import { name as appName } from './app.json';

AppRegistry.registerComponent(appName, () => App);
AppRegistry.registerComponent('Second', () => Second);
