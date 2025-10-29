import { Text, View, StyleSheet, Button } from 'react-native';
import { multiply, openNewWindow } from '../../src/index';

const result = multiply(3, 7);

export default function App() {
  return (
    <View style={styles.container}>
      <Text>Resultt: {result}</Text>
      <Button
        onPress={() => {
          const responseCode = openNewWindow();
          console.log('Response code from openNewWindow:', responseCode);
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
