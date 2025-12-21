import { useEffect, useState } from 'react';
import { DeviceEventEmitter, Platform } from 'react-native';
import LIB, { type WindowEvent } from './NativeMultiWindow';

const WINDOW_REGISTRY = {
  onOpen: (window: { title: string; id: number; openedAt: number }) => {
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

const OPEN_WINDOWS: Record<
  number,
  {
    title: string;
    id: number;
    openedAt: number;
    mainWindow?: boolean;
  }
> = {};

DeviceEventEmitter.addListener('MultiWindow/event', (event: WindowEvent) => {
  if (event.type === 764) {
    WINDOW_REGISTRY.onClose(event.id);
  }
  if (event.type === 9873) {
    WINDOW_REGISTRY.onOpen({
      id: event.id,
      title: event.title,
      openedAt: Date.now(),
      ...(Platform.OS === 'ios' && Object.keys(OPEN_WINDOWS).length === 0
        ? { mainWindow: true }
        : {}),
    });
  }
});

function getWindowList() {
  return Object.values(OPEN_WINDOWS).sort((a, b) => a.openedAt - b.openedAt);
}

export function useWindowList() {
  const [openWindows, setOpenWindows] = useState(getWindowList());

  useEffect(() => {
    if (Platform.OS === 'ios') {
      // On ipadOS, a new window can be created by the os UI, so a queue
      // of event is created at launch, but since turboModules are lazy loaded,
      // those events are not sent until the module is loaded, so we call
      // closeWindowBy with an invalid id to flush the queue.
      LIB.closeWindowBy(0);
    }
    const uppp = () => {
      setOpenWindows(getWindowList());
    };
    WINDOW_REGISTRY.subscribers.add(uppp);

    return () => {
      WINDOW_REGISTRY.subscribers.delete(uppp);
    };
  }, []);

  return openWindows;
}
