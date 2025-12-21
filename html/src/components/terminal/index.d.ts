import { Component, h } from 'preact';
import { XtermOptions } from './xterm';
import '@xterm/xterm/css/xterm.css';
interface Props extends XtermOptions {
    id: string;
}
interface State {
    modal: boolean;
    title: string;
    actionModal: 'none' | 'upload' | 'download';
    remotePath: string;
    writable: boolean;
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
    performDownload(): Promise<void>;
    render({ id }: Props, { modal, title, actionModal, remotePath }: State): h.JSX.Element;
    showModal(): void;
    sendFile(event: Event): void;
}
export {};
