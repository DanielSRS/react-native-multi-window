import {
  NativeModules,
  Platform,
  TurboModuleRegistry,
  type TurboModule,
} from 'react-native';

/**
 * Event emitted when a window is closed.
 * Contains the ID of the closed window.
 * This is emited when a window is closed successfully
 * no matter who initiated the close, either the user or programmatically
 * called closeWindowBy.
 */
export interface WindowClosedEvent {
  type: 764;
  id: number;
}

/**
 * Event emitted when a window is opened.
 * Contains the ID and title of the opened window.
 * This is emitted when a new window is successfully created
 * no matter who initiated the open, either the user
 * (like on ipadOS you can drag a window to create a new one
 * or even click the +) or programmatically
 * called openNewWindow.
 */
export interface WindowOpenedEvent {
  type: 9873;
  id: number;
  title: string;
}

export type WindowEvent = WindowClosedEvent | WindowOpenedEvent;

/**
 * Types of windows that can be created. this is specific to Windows platform.
 * - 0: Standard window
 * - 1: Acrylic window
 * - 2: Mica window
 */
export type WindowType = 0 | 1 | 2;

/**
 * Options for creating a new window.
 */
export interface WindowOptions {
  /**
   * The title of the new window.
   * Only shown an used on platforms that support window titles.
   */
  title: string;
  /**
   * The name of the React component to render in the new window.
   * This must match a registered component name in the AppRegistry.
   * e.g., AppRegistry.registerComponent('MyComponent', () => MyComponent);
   */
  componentName: string;
  /**
   * The type of window to create (specific to Windows platform).
   * - 0: Standard window
   * - 1: Acrylic window
   * - 2: Mica window
   * @default 0
   * @platform windows
   */
  windows_WindowType: WindowType;
  /**
   * Initial props to pass to the React component in the new window.
   */
  initialProps?: Object;
}

/**
 * Result of creating a new window.
 * Resolves to the window ID of the newly created window if successful.
 * Otherwise, it returns an error code.
 *
 * It never throws exceptions; errors are indicated via the returned value.
 */
type WindowCreationResult = Promise<number>;

export interface Spec extends TurboModule {
  openNewWindow(options: WindowOptions): WindowCreationResult;
  /**
   * Closes the window with the specified ID.
   * @param id The ID of the window to close.
   * @returns An integer status code indicating success or failure.
   * A positive value is the window ID that was closed and indicates success,
   * while a negative value indicates an error.
   */
  closeWindowBy(id: number): number;
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
