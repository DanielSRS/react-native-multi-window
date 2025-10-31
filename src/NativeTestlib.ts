import { TurboModuleRegistry, type TurboModule } from 'react-native';

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
  openNewWindow(): Promise<number>;
  openMicaWindow(): Promise<number>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Testlib');
