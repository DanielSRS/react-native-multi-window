import { Styled } from '@danielsrs/react-native-sdk';

export const WindowRow = Styled.createStyledView({
  borderRadius: 16,
  borderWidth: 1,
  padding: 16,
  marginBottom: 14,
  rowGap: 8,
});

export const WindowRowHeader = Styled.createStyledView({
  flexDirection: 'row',
  justifyContent: 'space-between',
  alignItems: 'flex-start',
  columnGap: 12,
});
