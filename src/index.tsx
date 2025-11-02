import Testlib from './NativeTestlib';
import type { WindowOptions } from './NativeTestlib';

export function multiply(a: number, b: number): number {
  return Testlib.multiply(a, b);
}

export function openNewWindow(windowOptions: WindowOptions): Promise<number> {
  return Testlib.openNewWindow(windowOptions);
}
