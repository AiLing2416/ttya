import type { ITerminalOptions } from '@xterm/xterm';
import { Terminal } from '@xterm/xterm';
import '@xterm/xterm/css/xterm.css';
interface TtydTerminal extends Terminal {
    fit(): void;
}
declare global {
    interface Window {
        term: TtydTerminal;
    }
}
export type RendererType = 'dom' | 'canvas' | 'webgl';
export interface ClientOptions {
    rendererType: RendererType;
    disableLeaveAlert: boolean;
    disableResizeOverlay: boolean;
    enableZmodem: boolean;
    enableTrzsz: boolean;
    enableSixel: boolean;
    titleFixed?: string;
    isWindows: boolean;
    trzszDragInitTimeout: number;
    unicodeVersion: string;
    closeOnDisconnect: boolean;
    title?: string;
    writable?: boolean;
}
export interface FlowControl {
    limit: number;
    highWater: number;
    lowWater: number;
}
export interface XtermOptions {
    wsUrl: string;
    tokenUrl: string;
    flowControl: FlowControl;
    clientOptions: ClientOptions;
    termOptions: ITerminalOptions;
}
export declare class Xterm {
    private options;
    private sendCb;
    private onTitle;
    private onWritable;
    private disposables;
    private textEncoder;
    private textDecoder;
    private written;
    private pending;
    private terminal;
    private fitAddon;
    private overlayAddon;
    private clipboardAddon;
    private webLinksAddon;
    private webglAddon?;
    private canvasAddon?;
    private zmodemAddon?;
    private socket?;
    private token;
    private opened;
    private title?;
    private titleFixed?;
    private resizeOverlay;
    private reconnect;
    private doReconnect;
    private closeOnDisconnect;
    private writeFunc;
    constructor(options: XtermOptions, sendCb: () => void, onTitle: (title: string) => void, onWritable: (writable: boolean) => void);
    dispose(): void;
    private register;
    sendFile(files: FileList): void;
    refreshToken(): Promise<void>;
    private onWindowUnload;
    open(parent: HTMLElement): void;
    private initListeners;
    writeData(data: string | Uint8Array): void;
    sendData(data: string | Uint8Array): void;
    connect(): void;
    private onSocketOpen;
    private onSocketClose;
    private parseOptsFromUrlQuery;
    private onSocketData;
    private applyPreferences;
    private setRendererType;
}
export {};
