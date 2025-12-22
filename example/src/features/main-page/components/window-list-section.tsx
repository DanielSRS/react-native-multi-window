import {
  Body,
  Button,
  Caption,
  Subtitle,
  useColors,
} from '@danielsrs/react-native-sdk';
import type { WindowType } from '../../../../../src';
import { View } from 'react-native';
import type { ViewStyle } from 'react-native';

import { MonoContainer, MonoContent } from '../../../components/mono-font';
import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import { WindowRow, WindowRowHeader } from '../../../components/window-row';
import { formatDuration } from '../../../shared/formatDuration';
import type { RegisteredWindow } from '../main-page';
import type { WindowMeta } from '../types';

export type WindowListSectionProps = {
  windows: RegisteredWindow[];
  windowMeta: Record<number, WindowMeta>;
  colors: ReturnType<typeof useColors>;
  cardSurface: ViewStyle;
  describeWindowType: (type: WindowType) => string;
  onCloseWindow: (id: number) => void;
};

export function WindowListSection({
  windows,
  windowMeta,
  colors,
  cardSurface,
  describeWindowType,
  onCloseWindow,
}: WindowListSectionProps) {
  return (
    <SectionCard style={cardSurface}>
      <SectionHeading
        title="Window list & live controls"
        apis="useWindowList + closeWindowBy"
        platform="All platforms"
        description="Enumerate every known window, derive durations, and close them remotely."
      />
      {windows.length === 0 && (
        <Body style={{ color: colors.fillColorTextSecondary }}>
          No secondary windows yet. Launch one above to populate this list.
        </Body>
      )}
      {windows.map((window) => {
        const meta = windowMeta[window.id];
        return (
          <WindowRow
            key={window.id}
            style={{
              borderColor: colors.strokeColorControlStrongStrokeDefault,
              backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
            }}
          >
            <WindowRowHeader>
              <View style={{ flex: 1 }}>
                <Subtitle>{window.title}</Subtitle>
                <Caption style={{ color: colors.fillColorTextSecondary }}>
                  id #{window.id} • opened{' '}
                  {formatDuration(Date.now() - window.openedAt)} ago
                </Caption>
                {window.mainWindow && (
                  <Caption style={{ color: colors.fillColorTextSecondary }}>
                    Main window reported by the platform.
                  </Caption>
                )}
              </View>
              <Button accent={false} onPress={() => onCloseWindow(window.id)}>
                Close
              </Button>
            </WindowRowHeader>
            <Body style={{ color: colors.fillColorTextSecondary }}>
              Component: {meta?.componentName ?? 'System managed / unknown'}
            </Body>
            <Body style={{ color: colors.fillColorTextSecondary }}>
              Window type:{' '}
              {meta ? describeWindowType(meta.type) : 'Not captured'}
            </Body>
            <Body style={{ color: colors.fillColorTextSecondary }}>
              Initial props
            </Body>
            <MonoContainer
              style={{
                borderColor: colors.strokeColorControlStrongStrokeDefault,
                backgroundColor: 'rgba(255,255,255,0.02)',
                maxHeight: 160,
              }}
            >
              <MonoContent selectable>
                {meta
                  ? JSON.stringify(meta.payload, null, 2)
                  : 'Initial props unknown for this window.'}
              </MonoContent>
            </MonoContainer>
          </WindowRow>
        );
      })}
    </SectionCard>
  );
}
