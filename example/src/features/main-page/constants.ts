import { WINDOW_TYPE, type WindowType } from '../../../../src';
import { name as appName } from '../../../app.json';
import type { DemoTarget } from './types';

export const WINDOW_LIST_COMPONENT_NAME = 'WindowListViewer';

export const ACCENT_PALETTE = [
  '#0F6CBD',
  '#8E8CD8',
  '#D13438',
  '#107C41',
  '#C239B3',
];

export const COMPONENT_TARGETS: Record<
  DemoTarget,
  {
    label: string;
    description: string;
    platform: string;
    componentName: string;
  }
> = {
  main: {
    label: 'Showcase component',
    description: 'Full demo surface rendered in auxiliary windows.',
    platform: 'All platforms',
    componentName: appName,
  },
  viewer: {
    label: 'Window list viewer',
    description: 'Compact diagnostic surface focused on window data.',
    platform: 'All platforms',
    componentName: WINDOW_LIST_COMPONENT_NAME,
  },
};

export const WINDOW_TYPE_DETAILS: Array<{
  type: WindowType;
  label: string;
  description: string;
  platform: string;
}> = [
  {
    type: WINDOW_TYPE.DEFAULT,
    label: 'Default frame',
    description: 'Standard OS chrome; works everywhere.',
    platform: 'All platforms',
  },
  {
    type: WINDOW_TYPE.ACRYLIC,
    label: 'Acrylic',
    description: 'WinUI acrylic material with blur and luminosity.',
    platform: 'Windows only',
  },
  {
    type: WINDOW_TYPE.MICA,
    label: 'Mica',
    description: 'WinUI Mica backdrop with dynamic tint.',
    platform: 'Windows only',
  },
];

export const TITLE_ADJECTIVES = [
  'Adaptive',
  'Connected',
  'Expressive',
  'Panoramic',
  'Dynamic',
  'Spatial',
  'Vivid',
];

export const TITLE_NOUNS = [
  'Workspace',
  'Canvas',
  'Panel',
  'Console',
  'Hub',
  'Studio',
];
