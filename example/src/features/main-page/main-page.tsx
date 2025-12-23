import { useCallback, useEffect, useMemo } from 'react';
import { ScrollView, View } from 'react-native';
import { Body, Caption, Title, useColors } from '@danielsrs/react-native-sdk';
import { closeWindowBy, useWindowList, type WindowType } from '../../../../src';

import { formatDuration } from '../../shared/formatDuration';
import { styles } from '../../shared/styles';
import {
  addWindowMeta,
  formatEvent,
  removeWindowMeta,
  useWindowDiagnostics,
  WINDOW_META,
} from '../events/useWindowDiagnostics';
import { WINDOW_TYPE_DETAILS } from './constants';
import { SharedCounterSection } from './components/shared-counter-section';
import { StatsSection, type StatEntry } from './components/stats-section';
import { WindowLauncherSection } from './components/window-launcher-section';
import { WindowListSection } from './components/window-list-section';
import { DiagnosticsSection } from './components/diagnostics-section';
import { useWindowBreakpoints } from '../../shared/use-bp';
import { useSelector } from '@legendapp/state/react';

export type RegisteredWindow = ReturnType<typeof useWindowList>[number];

export function MainPage({ rootProps }: { rootProps: unknown }) {
  const windows = useWindowList();
  const colors = useColors();
  const breakpoint = useWindowBreakpoints();
  const isWide = breakpoint >= breakpoint.LARGE_BREAKPOINT;

  const windowMeta = useSelector(WINDOW_META);

  useEffect(() => {
    console.log('Showcase multi-window props', rootProps);
  }, [rootProps]);

  const { eventLog, nativeLogs } = useWindowDiagnostics();

  const stats = useMemo(() => deriveWindowStats(windows), [windows]);

  const handleCloseWindow = useCallback((id: number) => {
    const result = closeWindowBy(id);
    if (result > 0) {
      removeWindowMeta(result);
    }
  }, []);

  const layoutStyle = isWide ? styles.row : styles.column;

  const statEntries: StatEntry[] = useMemo(
    () => [
      { label: 'Open windows', value: stats.total.toString() },
      {
        label: 'Main window id',
        value: stats.mainWindowId ? `#${stats.mainWindowId}` : 'Not reported',
      },
      { label: 'Average lifetime', value: stats.averageLifetime },
      { label: 'Longest alive', value: stats.longestLived },
      { label: 'Latest title', value: stats.newestTitle ?? 'Not reported' },
    ],
    [stats]
  );

  const sharedCardSurface = {
    backgroundColor: colors.backgroundFillColorLayerDefault,
    borderColor: colors.strokeColorControlStrongStrokeDefault,
  } as const;

  return (
    <ScrollView contentContainerStyle={styles.scrollContent}>
      <View style={{ rowGap: 8 }}>
        <Caption style={{ color: colors.fillColorTextSecondary }}>
          Showcase surface
        </Caption>
        <Title>Multi-window showcase</Title>
        <Body style={{ color: colors.fillColorTextSecondary }}>
          Each area highlights a specific API from react-native-multi-window and
          showcases the derived data that the app can calculate on top of it.
        </Body>
      </View>

      <View style={[layoutStyle, { columnGap: 20, rowGap: 20, marginTop: 20 }]}>
        <View style={styles.flexItem}>
          <SharedCounterSection
            colors={colors}
            cardSurface={sharedCardSurface}
          />
        </View>

        <View style={styles.flexItem}>
          <StatsSection
            statEntries={statEntries}
            colors={colors}
            cardSurface={sharedCardSurface}
          />
        </View>
      </View>

      <WindowLauncherSection colors={colors} setWindowMeta={addWindowMeta} />

      <WindowListSection
        windows={windows}
        windowMeta={windowMeta}
        colors={colors}
        cardSurface={sharedCardSurface}
        describeWindowType={describeWindowType}
        onCloseWindow={handleCloseWindow}
      />

      <DiagnosticsSection
        eventLog={eventLog}
        nativeLogs={nativeLogs}
        colors={colors}
        cardSurface={sharedCardSurface}
        formatEvent={formatEvent}
      />
    </ScrollView>
  );
}

interface WindowStats {
  total: number;
  mainWindowId: number | null;
  averageLifetime: string;
  longestLived: string;
  newestTitle: string | undefined;
}

function deriveWindowStats(windows: RegisteredWindow[]): WindowStats {
  if (windows.length === 0) {
    return {
      total: 0,
      mainWindowId: null,
      averageLifetime: '—',
      longestLived: '—',
      newestTitle: 'No windows yet',
    };
  }

  const durations = windows.map((window) => Date.now() - window.openedAt);
  const longest = Math.max(...durations);
  const average =
    durations.reduce((sum, value) => sum + value, 0) / durations.length;

  return {
    total: windows.length,
    mainWindowId: windows.find((window) => window.mainWindow)?.id ?? null,
    averageLifetime: formatDuration(Math.round(average)),
    longestLived: formatDuration(longest),
    newestTitle: windows[windows.length - 1]?.title,
  };
}

function describeWindowType(type: WindowType) {
  const found = WINDOW_TYPE_DETAILS.find((entry) => entry.type === type);
  return found ? `${found.label} (${found.platform})` : 'Unknown type';
}
