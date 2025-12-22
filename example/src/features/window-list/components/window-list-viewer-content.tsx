import { ScrollView } from 'react-native';
import {
  Body,
  Button,
  Caption,
  Subtitle,
  useColors,
} from '@danielsrs/react-native-sdk';
import { useWindowList } from '../../../../../src';

import { ButtonRow } from '../../../components/button-row';
import { MonoContainer, MonoContent } from '../../../components/mono-font';
import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import { WindowRow } from '../../../components/window-row';
import { formatDuration } from '../../../shared/formatDuration';
import { styles } from '../../../shared/styles';
import { useCounter } from '../../../useCounter';
import type { WindowPayload } from '../../main-page/types';

export type WindowListViewerProps = Partial<WindowPayload>;

export function WindowListViewerContent({
  initialPayload,
}: {
  initialPayload: WindowListViewerProps;
}) {
  const windows = useWindowList();
  const { count, increment } = useCounter();
  const colors = useColors();

  return (
    <ScrollView contentContainerStyle={styles.scrollContent}>
      <SectionCard
        style={{
          backgroundColor: colors.backgroundFillColorCardBackgroundSecondary,
          borderColor: colors.strokeColorControlStrongStrokeDefault,
        }}
      >
        <SectionHeading
          title="Window list viewer"
          apis="Registered as WindowListViewer component"
          platform="All platforms"
          description="Open this window to monitor shared state from a compact layout."
        />
        <Subtitle>Shared counter: {count}</Subtitle>
        <ButtonRow>
          <Button onPress={increment}>Increment counter</Button>
        </ButtonRow>
      </SectionCard>

      <SectionCard
        style={{
          backgroundColor: colors.backgroundFillColorCardBackgroundSecondary,
          borderColor: colors.strokeColorControlStrongStrokeDefault,
        }}
      >
        <SectionHeading
          title="Initial props received"
          apis="initialProps"
          platform="All platforms"
          description="Everything sent during openNewWindow arrives here."
        />
        <MonoContainer
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            backgroundColor: 'rgba(255,255,255,0.02)',
          }}
        >
          <MonoContent selectable>
            {Object.keys(initialPayload).length
              ? JSON.stringify(initialPayload, null, 2)
              : 'No initial props were provided.'}
          </MonoContent>
        </MonoContainer>
      </SectionCard>

      <SectionCard
        style={{
          backgroundColor: colors.backgroundFillColorCardBackgroundSecondary,
          borderColor: colors.strokeColorControlStrongStrokeDefault,
        }}
      >
        <SectionHeading
          title="Live window list"
          apis="useWindowList"
          platform="All platforms"
          description="The same hook reacts to open/close events in every window."
        />
        {windows.map((window) => (
          <WindowRow
            key={window.id}
            style={{
              borderColor: colors.strokeColorControlStrongStrokeDefault,
              backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
            }}
          >
            <Subtitle>{window.title}</Subtitle>
            <Caption style={{ color: colors.fillColorTextSecondary }}>
              id #{window.id} • opened{' '}
              {formatDuration(Date.now() - window.openedAt)} ago
            </Caption>
          </WindowRow>
        ))}
        {windows.length === 0 && (
          <Body style={{ color: colors.fillColorTextSecondary }}>
            No windows detected from this process yet.
          </Body>
        )}
      </SectionCard>
    </ScrollView>
  );
}
