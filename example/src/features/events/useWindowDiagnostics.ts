import { DeviceEventEmitter } from 'react-native';
import type { WindowEvent } from '../../../../src';
import { observable } from '@legendapp/state';
import { useSelector } from '@legendapp/state/react';
import type { WindowMeta } from '../main-page/types';

export type WindowEventLog = WindowEvent & { recordedAt: number };

export const WINDOW_META = observable<Record<number, WindowMeta>>({});

export function addWindowMeta(id: number, meta: WindowMeta) {
  WINDOW_META[id]?.set(meta);
}

export function removeWindowMeta(id: number) {
  WINDOW_META[id]?.delete();
}

const EVENT_LOGS = observable<WindowEventLog[]>([]);
const NATIVE_LOGS = observable<string[]>([]);

DeviceEventEmitter.addListener('MultiWindow/event', (event: WindowEvent) => {
  EVENT_LOGS.push({ ...event, recordedAt: Date.now() });
  if (event.type === 764) {
    removeWindowMeta(event.id);
  }
});

DeviceEventEmitter.addListener('MultiWindow/logs', (payload) => {
  const serialized =
    typeof payload === 'string' ? payload : JSON.stringify(payload);
  NATIVE_LOGS.push(`${new Date().toTimeString()} ${serialized}`);
});

export function useWindowDiagnostics(limit = 18) {
  const eventLog = useSelector(EVENT_LOGS).toReversed().slice(0, limit);
  const nativeLogs = useSelector(NATIVE_LOGS).toReversed().slice(0, limit);

  return { eventLog, nativeLogs };
}

export function formatEvent(event: WindowEventLog) {
  const timestamp = new Date(event.recordedAt).toTimeString();
  const t = event.type;
  switch (t) {
    case 4521:
      return `[${timestamp}] Focused window #${event.id}`;
    case 764:
      return `[${timestamp}] Closed window #${event.id}`;
    case 9873:
      return `[${timestamp}] Opened #${event.id} – ${event.title}`;
  }
}
