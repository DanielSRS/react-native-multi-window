import MultiWindow, { type WindowClosedEvent as TT } from './NativeMultiWindow';

export const openNewWindow = MultiWindow.openNewWindow;

export const closeWindowBy = MultiWindow.closeWindowBy;

export type WindowClosedEvent = TT;
