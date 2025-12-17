import {
  Text,
  View,
  StyleSheet,
  Button,
  DeviceEventEmitter,
} from 'react-native';
import { multiply, openNewWindow } from '../../src/index';
import { useEffect, useMemo, useState } from 'react';
import { name as appName } from '../app.json';

const result = multiply(3, 7);

DeviceEventEmitter.addListener('MultiWindow/logs', (event) => {
  console.log('Received multiWindowEvent:', event);
});

const up = {
  c: 0,
  fn: () => {
    up.c += 1;
    up.notify();
  },
  subscribers: new Set<() => void>(),
  notify: () => {
    up.subscribers.forEach((fn) => fn());
  },
};

export default function App() {
  const backgroundColor = useMemo(() => randomColor(), []);
  const [v, setv] = useState(1);

  useEffect(() => {
    const uppp = () => {
      setv((prev) => prev + 1);
    };
    up.subscribers.add(uppp);

    return () => {
      up.subscribers.delete(uppp);
    };
  }, []);

  return (
    <View style={[styles.container]}>
      <View style={[{ backgroundColor, height: 100, width: 100 }]} />
      <Text key={v}>count: {up.c}</Text>
      <Text>Resultt: {result}</Text>
      <Button
        onPress={async () => {
          up.fn();
        }}
        title="Count"
        color={'#0fb065ff'}
      />
      <Button
        onPress={async () => {
          try {
            const responseCode = await openNewWindow({
              title: 'Agora vai',
              componentName: appName,
              windows_WindowType: 0,
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
              title: 'Agora vai with mica?',
              componentName: appName,
              windows_WindowType: 2,
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
        title="multiply"
        onPress={() => {
          console.log('Multiply:', multiply(6, 7));
        }}
      />
    </View>
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
