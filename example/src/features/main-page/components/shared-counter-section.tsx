import { Button, Caption, Title, useColors } from '@danielsrs/react-native-sdk';

import { ButtonRow } from '../../../components/button-row';
import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import type { ViewStyle } from 'react-native';
import { useCounter } from '../../../useCounter';

export type SharedCounterSectionProps = {
  colors: ReturnType<typeof useColors>;
  cardSurface: ViewStyle;
};

export function SharedCounterSection({
  colors,
  cardSurface,
}: SharedCounterSectionProps) {
  const { count, increment } = useCounter();
  return (
    <SectionCard style={cardSurface}>
      <SectionHeading
        title="Shared counter"
        apis="useCounter (shared store)"
        platform="All platforms"
        description="Updated anywhere, synchronized everywhere."
      />
      <Title>{count}</Title>
      <Caption style={{ color: colors.fillColorTextSecondary }}>
        This value lives in a lightweight observable; every window receives
        updates instantly.
      </Caption>
      <ButtonRow>
        <Button onPress={increment}>Increment counter</Button>
      </ButtonRow>
    </SectionCard>
  );
}
