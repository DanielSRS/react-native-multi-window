import { Text, View, StyleSheet, Button } from 'react-native';
import { multiply, openNewWindow } from '../../src/index';

const result = multiply(3, 7);

export default function App() {
  return (
    <View style={styles.container}>
      <Text>Resultt: {result}</Text>
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
