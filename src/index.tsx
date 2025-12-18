import MultiWindow from './NativeMultiWindow';

export function multiply(a: number, b: number): number {
  return MultiWindow.multiply(a, b);
}

export const openNewWindow = MultiWindow.openNewWindow;
