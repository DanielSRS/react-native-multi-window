import {
  Text,
  View,
  StyleSheet,
  Button,
  DeviceEventEmitter,
} from 'react-native';
import { closeWindowBy, openNewWindow } from '../../src/index';
import { useCallback, useEffect, useMemo, useState } from 'react';
import { name as appName } from '../app.json';

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

function useCounter() {
  const [count, setCount] = useState(up.c);

  useEffect(() => {
    const uppp = () => {
      setCount(up.c);
    };
    up.subscribers.add(uppp);

    return () => {
      up.subscribers.delete(uppp);
    };
  }, []);

  const increment = useCallback(() => {
    up.fn();
  }, []);

  return { count, increment };
}

const OPEN_WINDOWS: Record<
  number,
  {
    title: string;
    id: number;
  }
> = {};

const WINDOW_REGISTRY = {
  onOpen: (window: { title: string; id: number }) => {
    OPEN_WINDOWS[window.id] = window;
    WINDOW_REGISTRY.notify();
  },
  onClose: (id: number) => {
    delete OPEN_WINDOWS[id];
    WINDOW_REGISTRY.notify();
  },
  subscribers: new Set<() => void>(),
  notify: () => {
    WINDOW_REGISTRY.subscribers.forEach((fn) => fn());
  },
};

function useWindowList() {
  const [openWindows, setOpenWindows] = useState(Object.values(OPEN_WINDOWS));

  useEffect(() => {
    const uppp = () => {
      setOpenWindows(Object.values(OPEN_WINDOWS));
    };
    WINDOW_REGISTRY.subscribers.add(uppp);

    return () => {
      WINDOW_REGISTRY.subscribers.delete(uppp);
    };
  }, []);

  const add = useCallback((window: { title: string; id: number }) => {
    WINDOW_REGISTRY.onOpen(window);
  }, []);

  const close = useCallback((id: number) => {
    WINDOW_REGISTRY.onClose(id);
  }, []);

  return { openWindows, add, close };
}

export default function App() {
  const backgroundColor = useMemo(() => randomColor(), []);
  const { count, increment } = useCounter();
  const {
    openWindows: windows,
    add: addWindow,
    close: closeWindow,
  } = useWindowList();

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
                windows_WindowType: 0,
              });
              console.log('Response code from openNewWindow:', responseCode);
              if (responseCode > 0) {
                addWindow({ id: responseCode, title });
              }
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
                windows_WindowType: 1,
              });
              console.log(
                'Response code from openNewWindow (acrylic):',
                responseCode
              );
              if (responseCode > 0) {
                addWindow({ id: responseCode, title: 'Acrylic vibes' });
              }
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
                windows_WindowType: 2,
              });
              console.log('Response code from openNewWindow:', responseCode);
              if (responseCode > 0) {
                addWindow({ id: responseCode, title: 'Agora vai with mica?' });
              }
            } catch (error) {
              console.error('Failed to open new window:', error);
            }
          }}
          title="Open New Window with mica effect"
          color={'#fcfc1eff'}
        />
        <Button
          title="Second"
          onPress={async () => {
            const responseCode = await openNewWindow({
              title: 'Second Window',
              componentName: 'Second',
              windows_WindowType: 0,
            });
            if (responseCode > 0) {
              addWindow({ id: responseCode, title: 'Second Window' });
            }
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
                    closeWindow(window.id);
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
