import { TurboModuleRegistry, type TurboModule } from 'react-native';

export interface WindowOptions {
  title: string;
}

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
  openNewWindow(options: WindowOptions): Promise<number>;
  openMicaWindow(): Promise<number>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Testlib');
