import { SdkProvider } from '@danielsrs/react-native-sdk';
import {
  WindowListViewerContent,
  type WindowListViewerProps,
} from './components/window-list-viewer-content';

export function WindowListViewer(props: WindowListViewerProps) {
  return (
    <SdkProvider>
      <WindowListViewerContent initialPayload={props} />
    </SdkProvider>
  );
}
