import {
  NativeModules,
  Platform,
  TurboModuleRegistry,
  type TurboModule,
} from 'react-native';

export interface Spec extends TurboModule {
  multiply(a: number, b: number): number;
}

/**
 * Gets the appropriate MultiWindow native module depending on whether
 * TurboModules are enabled or not.
 */
const MultiWindowTurboModule = TurboModuleRegistry.get<Spec>('MultiWindow');
const MultiWindowLegacy = NativeModules.MultiWindow as Spec | null;

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

if (!MultiWindowTurboModule && !MultiWindowLegacy) {
  throw new Error(LINKING_ERROR);
}

export default MultiWindowTurboModule ?? MultiWindowLegacy!;
