import { useEffect, useState } from 'react';
import { DeviceEventEmitter } from 'react-native';
import type { WindowEvent } from '../../../../src';

export type WindowEventLog = WindowEvent & { recordedAt: number };

export function useWindowDiagnostics(
  onWindowClosed: (id: number) => void,
  limit = 18
) {
  const [eventLog, setEventLog] = useState<WindowEventLog[]>([]);
  const [nativeLogs, setNativeLogs] = useState<string[]>([]);

  useEffect(() => {
    const eventSubscription = DeviceEventEmitter.addListener(
      'MultiWindow/event',
      (event: WindowEvent) => {
        setEventLog((previous) =>
          [{ ...event, recordedAt: Date.now() }, ...previous].slice(0, limit)
        );
        if (event.type === 764) {
          onWindowClosed(event.id);
        }
      }
    );

    const logSubscription = DeviceEventEmitter.addListener(
      'MultiWindow/logs',
      (payload) => {
        const serialized =
          typeof payload === 'string' ? payload : JSON.stringify(payload);
        setNativeLogs((previous) =>
          [
            `${new Date().toLocaleTimeString()} ${serialized}`,
            ...previous,
          ].slice(0, limit)
        );
      }
    );

    return () => {
      eventSubscription.remove();
      logSubscription.remove();
    };
  }, [limit, onWindowClosed]);

  return { eventLog, nativeLogs };
}

type LocalWindowEvent = {
  type: number;
  id: number;
  title?: string;
  recordedAt: number;
};

export function formatEvent(event: WindowEventLog | LocalWindowEvent) {
  const timestamp = new Date((event as any).recordedAt).toLocaleTimeString();
  const t = (event as any).type as number;
  if (t === 9873) {
    return `[${timestamp}] Opened #${(event as any).id} – ${(event as any).title}`;
  }
  if (t === 764) {
    return `[${timestamp}] Closed window #${(event as any).id}`;
  }
  return `[${timestamp}] Event ${t} for window #${(event as any).id}`;
}
