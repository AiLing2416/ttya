import { h, Component, ComponentChildren } from 'preact';
import './modal.scss';
interface Props {
    show: boolean;
    children: ComponentChildren;
}
export declare class Modal extends Component<Props> {
    constructor(props: Props);
    render({ show, children }: Props): false | h.JSX.Element;
}
export {};
