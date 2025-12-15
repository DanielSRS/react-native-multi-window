import {
  NativeModules,
  Platform,
  TurboModuleRegistry,
  type TurboModule,
} from 'react-native';

const LINKING_ERROR =
  `The package 'react-native-multi-window' doesn't seem to be linked. Make sure:
\n\n` +
  Platform.select({
    macos:
      "  • You ran 'pod install' inside the macOS workspace\n" +
      '  • You rebuilt the app after installing the pods\n',
    default:
      '  • You installed the library\n' +
      '  • You rebuilt the app after installing the pods or packages\n',
  });

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
}

const MultiWindowTurboModule = TurboModuleRegistry.get<Spec>('MultiWindow');

const MultiWindowLegacy = NativeModules.MultiWindow as Spec | null;

if (!MultiWindowTurboModule && !MultiWindowLegacy) {
  throw new Error(LINKING_ERROR);
}

export default MultiWindowTurboModule ?? MultiWindowLegacy!;
