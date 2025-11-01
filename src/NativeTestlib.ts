import { TurboModuleRegistry, type TurboModule } from 'react-native';

export interface WindowOptions {
  title: string;
  /**
   * Type of window to create. windows only property.
   *
   * 0 - Default window
   * 1 - Mica window
   * 2 - Default window with Mica effect applied
   */
  windows_WindowType: 0 | 1 | 2;
}

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
  openNewWindow(options: WindowOptions): Promise<number>;
  openMicaWindow(): Promise<number>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Testlib');
