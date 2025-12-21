import {
  Text,
  View,
  StyleSheet,
  Button,
  DeviceEventEmitter,
} from 'react-native';
import {
  closeWindowBy,
  openNewWindow,
  useWindowList,
  WINDOW_TYPE,
} from '../../src/index';
import { useEffect, useMemo } from 'react';
import { name as appName } from '../app.json';
import { useCounter } from './useCounter';

DeviceEventEmitter.addListener('MultiWindow/logs', (event) => {
  console.log('EVENT:', event, 'typeof event: ', typeof event);
});

export default function App(props: unknown) {
  const backgroundColor = useMemo(() => randomColor(), []);
  const { count, increment } = useCounter();
  const windows = useWindowList();

  useEffect(() => {
    console.log('!!!!!!!App props:', props);
    // force the module to be load
  }, [props]);

  return (
    <View style={{ flex: 1, flexDirection: 'row' }}>
      <View style={[styles.container]}>
        <View style={[{ backgroundColor, height: 100, width: 100 }]} />
        <Text key={count}>count: {count}</Text>
        <Button onPress={increment} title="Increment" color={'#0fb065ff'} />
        <Button
          onPress={async () => {
            const title = 'New Window ' + Math.floor(Math.random() * 1000);
            try {
              const responseCode = await openNewWindow({
                title,
                componentName: appName,
                windows_WindowType: WINDOW_TYPE.DEFAULT,
              });
              console.log('Response code from openNewWindow:', responseCode);
            } catch (error) {
              console.error('Failed to open new window:', error);
            }
          }}
          title="Open New Window"
          color={'#841584'}
        />
        <Button
          onPress={async () => {
            try {
              const responseCode = await openNewWindow({
                title: 'Acrylic vibes',
                componentName: appName,
                windows_WindowType: WINDOW_TYPE.ACRYLIC,
              });
              console.log(
                'Response code from openNewWindow (acrylic):',
                responseCode
              );
            } catch (error) {
              console.error('Failed to open acrylic window:', error);
            }
          }}
          title="Open Window with acrylic effect"
          color={'#4bc0f8'}
        />
        <Button
          onPress={async () => {
            try {
              const responseCode = await openNewWindow({
                title: 'Agora vai with mica?',
                componentName: appName,
                windows_WindowType: WINDOW_TYPE.MICA,
                initialProps: {
                  info: 'This is a mica window ;;;',
                },
              });
              console.log('Response code from openNewWindow:', responseCode);
            } catch (error) {
              console.error('Failed to open new window:', error);
            }
          }}
          title="Open New Window with mica effect"
          color={'#fcfc1eff'}
        />
        <Button
          title="Second"
          onPress={() => {
            openNewWindow({
              title: 'Second Window',
              componentName: 'Second',
              windows_WindowType: 0,
            });
          }}
        />
      </View>

      {/* Window list */}
      <View style={{ flex: 1, padding: 10 }}>
        {windows.map((window) => {
          return (
            <View
              key={window.id}
              style={{
                flexDirection: 'row',
                justifyContent: 'space-between',
              }}
            >
              <Text>
                {window.id} - {window.title}
              </Text>
              <Button
                title="Close"
                onPress={() => {
                  const res = closeWindowBy(window.id);
                  if (res > 0) {
                    // closeWindow(window.id);
                  }
                }}
              />
            </View>
          );
        })}
      </View>
    </View>
  );
}

export function Second() {
  const backgroundColor = useMemo(() => randomColor(), []);
  return (
    <View
      style={{
        flex: 1,
        backgroundColor,
      }}
    />
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});

function randomColor() {
  const r = Math.floor(Math.random() * 256);
  const g = Math.floor(Math.random() * 256);
  const b = Math.floor(Math.random() * 256);
  return `rgb(${r},${g},${b})`;
}
