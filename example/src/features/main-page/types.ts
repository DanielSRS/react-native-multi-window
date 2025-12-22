import type { WindowType } from '../../../../src';

export type DemoTarget = 'main' | 'viewer';

export type WindowPayload = {
  info: string;
  accentColor: string;
  dataset: Array<{ label: string; value: number }>;
  tags: string[];
  payloadSize: number;
  generatedAt: number;
  target: DemoTarget;
  hostPlatform: string;
};

export type WindowMeta = {
  componentKey: DemoTarget;
  componentName: string;
  payload: WindowPayload;
  type: WindowType;
  title: string;
  openedAt: number;
};

export type WindowLaunchConfig = {
  title: string;
  componentKey: DemoTarget;
  componentName: string;
  type: WindowType;
  payload: WindowPayload;
};
