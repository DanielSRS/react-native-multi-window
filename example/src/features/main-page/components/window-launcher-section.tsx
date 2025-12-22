import {
  Button,
  Caption,
  RadioButton,
  Slider,
  useColors,
} from '@danielsrs/react-native-sdk';
import {
  openNewWindow,
  WINDOW_TYPE,
  type WindowType,
} from '../../../../../src';
import { Platform, View } from 'react-native';

import { ButtonRow } from '../../../components/button-row';
import { MonoContainer, MonoContent } from '../../../components/mono-font';
import { SectionCard } from '../../../components/section-card';
import { SectionHeading } from '../../../components/section-heading';
import {
  ACCENT_PALETTE,
  COMPONENT_TARGETS,
  TITLE_ADJECTIVES,
  TITLE_NOUNS,
  WINDOW_TYPE_DETAILS,
} from '../constants';
import type {
  DemoTarget,
  WindowLaunchConfig,
  WindowMeta,
  WindowPayload,
} from '../types';
import {
  InlineStack,
  InputsGrid,
  PaletteRow,
  ShowcaseInput,
  SwatchButton,
} from './feature-primitives';
import React, { useCallback, useMemo, useState } from 'react';

export type WindowLauncherSectionProps = {
  colors: ReturnType<typeof useColors>;
  setWindowMeta: (
    data: React.SetStateAction<Record<number, WindowMeta>>
  ) => void;
};

export function WindowLauncherSection({
  colors,
  setWindowMeta,
}: WindowLauncherSectionProps) {
  const [windowTitle, setWindowTitle] = useState(() => generateWindowTitle());
  const [notes, setNotes] = useState(
    'Curate multi-window experiences while sharing the same data model.'
  );
  const [dataPoints, setDataPoints] = useState(3);
  const [selectedAccentIndex, setSelectedAccentIndex] = useState(0);
  const [selectedType, setSelectedType] = useState<WindowType>(
    WINDOW_TYPE.DEFAULT
  );
  const [selectedTarget, setSelectedTarget] = useState<DemoTarget>('main');

  const accentColor = ACCENT_PALETTE[
    selectedAccentIndex % ACCENT_PALETTE.length
  ] as string;

  const payloadPreview = useMemo(
    () =>
      buildWindowPayload({
        info: notes,
        accentColor,
        dataPoints,
        target: selectedTarget,
      }),
    [notes, accentColor, dataPoints, selectedTarget]
  );
  const handleWindowOpen = useCallback(
    async (override?: Partial<WindowLaunchConfig>) => {
      const componentKey = override?.componentKey ?? selectedTarget;
      const request: WindowLaunchConfig = {
        title: override?.title ?? windowTitle,
        componentKey,
        componentName:
          override?.componentName ??
          COMPONENT_TARGETS[componentKey].componentName,
        type: override?.type ?? selectedType,
        payload: override?.payload ?? payloadPreview,
      };

      try {
        const responseCode = await openNewWindow({
          title: request.title,
          componentName: request.componentName,
          windows_WindowType: request.type,
          initialProps: request.payload,
        });

        if (responseCode > 0) {
          setWindowMeta((previous) => ({
            ...previous,
            [responseCode]: {
              componentKey: request.componentKey,
              componentName: request.componentName,
              payload: request.payload,
              type: request.type,
              title: request.title,
              openedAt: Date.now(),
            },
          }));
        } else {
          console.warn('openNewWindow returned a non-window id:', responseCode);
        }
      } catch (error) {
        console.error('Failed to open window', error);
      }
    },
    [payloadPreview, selectedTarget, selectedType, windowTitle, setWindowMeta]
  );
  const handleOpenDiagnostics = useCallback(() => {
    handleWindowOpen({
      componentKey: 'viewer',
      componentName: COMPONENT_TARGETS.viewer.componentName,
      payload: buildWindowPayload({
        info: 'Dedicated window list viewer',
        accentColor,
        dataPoints,
        target: 'viewer',
      }),
      title: 'Window List Monitor',
    });
  }, [accentColor, dataPoints, handleWindowOpen]);

  const handleOpenAcrylic = useCallback(() => {
    handleWindowOpen({
      type: WINDOW_TYPE.ACRYLIC,
      payload: buildWindowPayload({
        info: 'Acrylic shell requested (Windows only).',
        accentColor,
        dataPoints,
        target: selectedTarget,
      }),
    });
  }, [accentColor, dataPoints, handleWindowOpen, selectedTarget]);

  const handleOpenMica = useCallback(() => {
    handleWindowOpen({
      type: WINDOW_TYPE.MICA,
      payload: buildWindowPayload({
        info: 'Mica shell requested (Windows only).',
        accentColor,
        dataPoints,
        target: selectedTarget,
      }),
    });
  }, [accentColor, dataPoints, handleWindowOpen, selectedTarget]);
  return (
    <SectionCard
      style={{
        backgroundColor: colors.backgroundFillColorLayerDefault,
        borderColor: colors.strokeColorControlStrongStrokeDefault,
      }}
    >
      <SectionHeading
        title="Window launcher"
        apis="openNewWindow + WINDOW_TYPE + initialProps"
        platform="Cross-platform & Windows-only effects"
        description="Configure a payload, select the component, and launch tuned secondary windows."
      />
      <InputsGrid>
        <ShowcaseInput
          value={windowTitle}
          onChangeText={setWindowTitle}
          placeholder="Window title"
          placeholderTextColor={colors.fillColorTextSecondary}
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            color: colors.fillColorTextPrimary,
            backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
          }}
        />
        <ShowcaseInput
          value={notes}
          onChangeText={setNotes}
          placeholder="Initial props summary"
          placeholderTextColor={colors.fillColorTextSecondary}
          multiline
          style={{
            borderColor: colors.strokeColorControlStrongStrokeDefault,
            color: colors.fillColorTextPrimary,
            backgroundColor: colors.backgroundFillColorCardBackgroundDefault,
            minHeight: 60,
          }}
        />

        <Caption style={{ color: colors.fillColorTextSecondary }}>
          Accent color • Showcase palette
        </Caption>
        <PaletteRow>
          {ACCENT_PALETTE.map((paletteColor, index) => (
            <SwatchButton
              key={paletteColor}
              onPress={() => setSelectedAccentIndex(index)}
              style={{
                backgroundColor: paletteColor,
                borderColor:
                  index === selectedAccentIndex
                    ? colors.accentDefault
                    : 'rgba(255,255,255,0.25)',
              }}
            />
          ))}
        </PaletteRow>

        <Caption style={{ color: colors.fillColorTextSecondary }}>
          Initial dataset size: {dataPoints} metrics
        </Caption>
        <Slider
          minimumValue={1}
          maximumValue={8}
          onValueChange={(value) => {
            setDataPoints(Math.max(1, Math.round(value)));
          }}
        />

        <Caption style={{ color: colors.fillColorTextSecondary }}>
          Target component
        </Caption>
        <InlineStack>
          {(Object.keys(COMPONENT_TARGETS) as Array<DemoTarget>).map(
            (targetKey) => (
              <View
                key={`${targetKey}-${selectedTarget === targetKey}`}
                style={{ maxWidth: 240 }}
              >
                <RadioButton
                  selected={selectedTarget === targetKey}
                  label={`${COMPONENT_TARGETS[targetKey].label} (${COMPONENT_TARGETS[targetKey].platform})`}
                  onPress={() => setSelectedTarget(targetKey)}
                />
                <Caption style={{ color: colors.fillColorTextSecondary }}>
                  {COMPONENT_TARGETS[targetKey].description}
                </Caption>
              </View>
            )
          )}
        </InlineStack>

        <Caption style={{ color: colors.fillColorTextSecondary }}>
          Window effect / platform scope
        </Caption>
        <InlineStack>
          {WINDOW_TYPE_DETAILS.map((option) => (
            <View
              key={`${option.type}-${selectedType === option.type}`}
              style={{ maxWidth: 240 }}
            >
              <RadioButton
                selected={selectedType === option.type}
                label={`${option.label} (${option.platform})`}
                onPress={() => setSelectedType(option.type)}
              />
              <Caption style={{ color: colors.fillColorTextSecondary }}>
                {option.description}
              </Caption>
            </View>
          ))}
        </InlineStack>
      </InputsGrid>

      <ButtonRow>
        <Button onPress={() => handleWindowOpen()}>
          Open configured window
        </Button>
        <Button
          accent={false}
          onPress={() => setWindowTitle(generateWindowTitle())}
        >
          Surprise me with a title
        </Button>
        <Button accent={false} onPress={handleOpenDiagnostics}>
          Launch diagnostics window
        </Button>
        <Button onPress={handleOpenAcrylic}>Acrylic effect (Windows)</Button>
        <Button onPress={handleOpenMica}>Mica effect (Windows)</Button>
      </ButtonRow>

      <Caption style={{ color: colors.fillColorTextSecondary }}>
        Initial props preview (sent to every new window)
      </Caption>
      <MonoContainer
        style={{
          borderColor: colors.strokeColorControlStrongStrokeDefault,
          backgroundColor: 'rgba(255,255,255,0.02)',
        }}
      >
        <MonoContent selectable>
          {JSON.stringify(payloadPreview, null, 2)}
        </MonoContent>
      </MonoContainer>
    </SectionCard>
  );
}

function generateWindowTitle() {
  const adjective =
    TITLE_ADJECTIVES[Math.floor(Math.random() * TITLE_ADJECTIVES.length)];
  const noun = TITLE_NOUNS[Math.floor(Math.random() * TITLE_NOUNS.length)];
  return `${adjective} ${noun}`;
}

function buildWindowPayload(params: {
  info: string;
  accentColor: string;
  dataPoints: number;
  target: DemoTarget;
}): WindowPayload {
  const dataset = Array.from({ length: Math.max(1, params.dataPoints) }).map(
    (_, index) => ({
      label: `Metric ${index + 1}`,
      value: Math.round(Math.random() * 100),
    })
  );

  return {
    info: params.info,
    accentColor: params.accentColor,
    dataset,
    tags: [params.target, Platform.OS, `points-${dataset.length}`],
    payloadSize: dataset.length,
    generatedAt: Date.now(),
    target: params.target,
    hostPlatform: Platform.OS,
  };
}
