import { SdkProvider } from '@danielsrs/react-native-sdk';
import { MainPage } from './features/main-page/main-page';
import { WindowListViewer } from './features/window-list/window-list-viewer';

/**
 * App
 *
 * Root component for the example application. This component contains only
 * pure components and delegates side-effects to the SDK provider which is
 * provided by the wrapper `react-native-sdk-wrapper` module.
 */
export default function App(props: unknown) {
  return (
    <SdkProvider>
      <MainPage rootProps={props} />
    </SdkProvider>
  );
}

export { WindowListViewer };
