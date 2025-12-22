import {
  Body,
  Caption,
  Subtitle,
  Styled,
  useColors,
} from '@danielsrs/react-native-sdk';
import { View } from 'react-native';

const PlatformPill = Styled.createStyledView({
  borderRadius: 999,
  borderWidth: 1,
  paddingHorizontal: 12,
  paddingVertical: 4,
});

export function SectionHeading({
  title,
  apis,
  platform,
  description,
}: {
  title: string;
  apis: string;
  platform: string;
  description?: string;
}) {
  const colors = useColors();
  return (
    <View style={{ rowGap: 6 }}>
      <Caption style={{ color: colors.fillColorTextSecondary }}>{apis}</Caption>
      <View
        style={{
          flexDirection: 'row',
          justifyContent: 'space-between',
          alignItems: 'flex-start',
          columnGap: 12,
        }}
      >
        <Subtitle>{title}</Subtitle>
        <PlatformPill
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
          }}
        >
          <Caption>{platform}</Caption>
        </PlatformPill>
      </View>
      {description && (
        <Body style={{ color: colors.fillColorTextSecondary }}>
          {description}
        </Body>
      )}
    </View>
  );
}
