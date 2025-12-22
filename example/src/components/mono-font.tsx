import { Body, Styled } from '@danielsrs/react-native-sdk';
import { Platform } from 'react-native';

const MONO_FONT =
  Platform.select({
    ios: 'Menlo',
    android: 'monospace',
    default: 'Courier New',
  }) ?? 'Courier New';

export const MonoContainer = Styled.createStyledView({
  borderRadius: 12,
  borderWidth: 1,
  padding: 12,
  marginTop: 8,
});

export const MonoContent = Styled.createStyled(Body, {
  fontFamily: MONO_FONT,
  fontSize: 12,
  lineHeight: 16,
});
