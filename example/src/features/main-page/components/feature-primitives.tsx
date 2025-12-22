import { Caption, Styled, Subtitle } from '@danielsrs/react-native-sdk';
import { TextInput } from 'react-native';

export const InlineStack = Styled.createStyledView({
  flexDirection: 'row',
  flexWrap: 'wrap',
  columnGap: 12,
  rowGap: 6,
  alignItems: 'center',
});

export const Pill = Styled.createStyledView({
  borderRadius: 999,
  borderWidth: 1,
  paddingVertical: 10,
  paddingHorizontal: 16,
  minWidth: 120,
});

export const PillLabel = Styled.createStyled(Caption, {
  textTransform: 'uppercase',
  fontSize: 11,
  letterSpacing: 0.6,
});

export const PillValue = Styled.createStyled(Subtitle, {
  fontSize: 20,
});

export const PaletteRow = Styled.createStyledView({
  flexDirection: 'row',
  columnGap: 12,
});

export const SwatchButton = Styled.createStyledTouchableOpacity({
  width: 36,
  height: 36,
  borderRadius: 18,
  borderWidth: 2,
});

export const InputsGrid = Styled.createStyledView({
  rowGap: 12,
});

export const ShowcaseInput = Styled.createStyled(TextInput, {
  borderWidth: 1,
  borderRadius: 12,
  paddingHorizontal: 14,
  paddingVertical: 10,
  fontSize: 16,
});
