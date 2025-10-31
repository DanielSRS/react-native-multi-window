import Testlib from './NativeTestlib';

export function multiply(a: number, b: number): number {
  return Testlib.multiply(a, b);
}

export function openNewWindow(): Promise<number> {
  return Testlib.openNewWindow();
}

export function openMicaWindow(): Promise<number> {
  return Testlib.openMicaWindow();
}
