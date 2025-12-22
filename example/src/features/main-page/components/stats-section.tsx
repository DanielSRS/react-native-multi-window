import { useColors } from '@danielsrs/react-native-sdk';
import type { ViewStyle } from 'react-native';

import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import { InlineStack, Pill, PillLabel, PillValue } from './feature-primitives';

export type StatEntry = {
  label: string;
  value: string;
};

export type StatsSectionProps = {
  statEntries: StatEntry[];
  colors: ReturnType<typeof useColors>;
  cardSurface: ViewStyle;
};

export function StatsSection({
  statEntries,
  colors,
  cardSurface,
}: StatsSectionProps) {
  return (
    <SectionCard style={cardSurface}>
      <SectionHeading
        title="Derived window stats"
        apis="useWindowList + derived metrics"
        platform="All platforms"
        description="The native API exposes ids and titles — the rest is derived here."
      />
      <InlineStack>
        {statEntries.map((entry) => (
          <Pill
            key={entry.label}
            style={{
              borderColor: colors.strokeColorControlStrongStrokeDefault,
              backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
            }}
          >
            <PillLabel style={{ color: colors.fillColorTextSecondary }}>
              {entry.label}
            </PillLabel>
            <PillValue>{entry.value}</PillValue>
          </Pill>
        ))}
      </InlineStack>
    </SectionCard>
  );
}
