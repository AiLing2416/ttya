import { Component, h } from 'preact';
import { XtermOptions } from './xterm';
import '@xterm/xterm/css/xterm.css';
interface Props extends XtermOptions {
    id: string;
}
export interface UploadState {
    id: string;
    filename: string;
    loaded: number;
    total: number;
    status: 'pending' | 'uploading' | 'success' | 'error';
    errorMsg?: string;
}
interface State {
    modal: boolean;
    title: string;
    actionModal: 'none' | 'upload' | 'download';
    remotePath: string;
    writable: boolean;
    uploads: UploadState[];
}
export declare class Terminal extends Component<Props, State> {
    private container;
    private xterm;
    constructor(props: Props);
    componentDidMount(): Promise<void>;
    componentWillUnmount(): void;
    onTitleChange(title: string): void;
    onWritableChange(writable: boolean): void;
    handleUploadClick(): void;
    handleDownloadClick(): void;
    closeActionModal(): void;
    handlePathChange(e: Event): void;
    performUpload(e: Event): Promise<void>;
    uploadSingleFile(file: File, remotePathBase: string, uploadId: string): void;
    updateUploadStatus(id: string, updates: Partial<{
        loaded: number;
        total: number;
        status: 'pending' | 'uploading' | 'success' | 'error';
        errorMsg: string;
    }>): void;
    clearUpload(id: string): void;
    performDownload(): Promise<void>;
    render({ id }: Props, { modal, title, actionModal, remotePath, uploads }: State): h.JSX.Element;
    showModal(): void;
    sendFile(event: Event): void;
}
export {};
