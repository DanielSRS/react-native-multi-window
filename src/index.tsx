import MultiWindow, { type WindowEvent as TT } from './NativeMultiWindow';

export const openNewWindow = MultiWindow.openNewWindow;

export const closeWindowBy = MultiWindow.closeWindowBy;

export type WindowEvent = TT;
