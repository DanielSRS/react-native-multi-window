import { TurboModuleRegistry, type TurboModule } from 'react-native';

export interface WindowOptions {
  title: string;
  /**
   * Type of window to create. Windows-only property.
   *
   * 0 - Default window
   * 1 - (deprecated) treated the same as 2 for backward compatibility
   * 2 - Default window with Mica effect applied
   */
  windows_WindowType: 0 | 1 | 2;
}

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
  openNewWindow(options: WindowOptions): Promise<number>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Testlib');
