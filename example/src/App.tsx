import {
  Text,
  View,
  StyleSheet,
  Button,
  DeviceEventEmitter,
} from 'react-native';
import { multiply } from '../../src';
import { useEffect } from 'react';

const result = multiply(3, 45);

export default function App() {
  useEffect(() => {
    const sub = DeviceEventEmitter.addListener('MultiWindow/logs', (event) => {
      console.log('Received multiWindowEvent:', event);
    });
    return () => {
      sub.remove();
    };
  }, []);
  return (
    <View style={styles.container}>
      <Text>Result: {result}</Text>
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
