import { h, Component } from 'preact';
interface Props {
    title: string;
    onUploadClick: () => void;
    onDownloadClick: () => void;
    writable?: boolean;
}
export declare class TopBar extends Component<Props> {
    render({ title, onUploadClick, onDownloadClick, writable }: Props): h.JSX.Element;
}
export {};
