import MultiWindow from './NativeMultiWindow';

export const openNewWindow = MultiWindow.openNewWindow;

export const closeWindowBy = MultiWindow.closeWindowBy;

export { useWindowList } from './hooks';
export { WINDOW_TYPE } from './NativeMultiWindow';

export type {
  WindowEvent,
  WindowOptions,
  WindowType,
} from './NativeMultiWindow';
