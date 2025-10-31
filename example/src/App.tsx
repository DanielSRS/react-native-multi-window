import { Text, View, StyleSheet, Button } from 'react-native';
import { multiply, openMicaWindow, openNewWindow } from '../../src/index';
import { useEffect, useMemo, useState } from 'react';

const result = multiply(3, 7);

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
    <View style={[styles.container, { backgroundColor }]}>
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
            const responseCode = await openNewWindow();
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
            const responseCode = await openMicaWindow();
            console.log('Response code from openMicaWindow:', responseCode);
          } catch (error) {
            console.error('Failed to open mica window:', error);
          }
        }}
        title="Open Mica Window"
        color={'#307fb3ff'}
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
