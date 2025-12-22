import { Body, Caption, useColors } from '@danielsrs/react-native-sdk';
import type { ViewStyle } from 'react-native';

import { MonoContainer, MonoContent } from '../../../components/mono-font';
import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import type { WindowEventLog } from '../../events/useWindowDiagnostics';

export type DiagnosticsSectionProps = {
  eventLog: WindowEventLog[];
  nativeLogs: string[];
  colors: ReturnType<typeof useColors>;
  cardSurface: ViewStyle;
  formatEvent: (event: WindowEventLog) => string;
};

export function DiagnosticsSection({
  eventLog,
  nativeLogs,
  colors,
  cardSurface,
  formatEvent,
}: DiagnosticsSectionProps) {
  return (
    <SectionCard style={cardSurface}>
      <SectionHeading
        title="Event log & native diagnostics"
        apis="DeviceEventEmitter • WindowEvent"
        platform="All platforms"
        description="Listen to MultiWindow/event and MultiWindow/logs to monitor what the native side emits."
      />
      <Caption style={{ color: colors.fillColorTextSecondary }}>
        Window events
      </Caption>
      {eventLog.length === 0 && (
        <Body style={{ color: colors.fillColorTextSecondary }}>
          Waiting for MultiWindow/event payloads…
        </Body>
      )}
      {eventLog.slice(0, 8).map((event) => (
        <MonoContainer
          key={`${event.recordedAt}-${event.id}-${event.type}`}
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            backgroundColor: 'rgba(255,255,255,0.02)',
          }}
        >
          <MonoContent selectable>{formatEvent(event)}</MonoContent>
        </MonoContainer>
      ))}

      <Caption style={{ color: colors.fillColorTextSecondary, marginTop: 12 }}>
        Native logs
      </Caption>
      {nativeLogs.length === 0 && (
        <Body style={{ color: colors.fillColorTextSecondary }}>
          No logs yet. Native modules can emit MultiWindow/logs for extra
          context.
        </Body>
      )}
      {nativeLogs.slice(0, 8).map((entry, index) => (
        <MonoContainer
          key={`${entry}-${index}`}
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            backgroundColor: 'rgba(255,255,255,0.02)',
          }}
        >
          <MonoContent selectable>{entry}</MonoContent>
        </MonoContainer>
      ))}
    </SectionCard>
  );
}
