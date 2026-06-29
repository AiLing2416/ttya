import { h, Component } from 'preact';
interface Props {
    title: string;
    onUploadClick: () => void;
    onDownloadClick: () => void;
    writable?: boolean;
    uploads?: any[];
}
interface State {
    showDropdown: boolean;
}
export declare class TopBar extends Component<Props, State> {
    constructor();
    render({ title, onUploadClick, onDownloadClick, writable, uploads }: Props, { showDropdown }: State): h.JSX.Element;
}
export {};
