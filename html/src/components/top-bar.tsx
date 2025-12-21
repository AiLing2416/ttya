import { h, Component } from 'preact';

interface Props {
    title: string;
    onUploadClick: () => void;
    onDownloadClick: () => void;
    writable?: boolean;
}

export class TopBar extends Component<Props> {
    render({ title, onUploadClick, onDownloadClick, writable }: Props) {
        const isWritable = writable !== false; // default true if undefined
        const btnStyle = {
            backgroundColor: isWritable ? '#333' : '#2a2a2a',
            color: isWritable ? '#fff' : '#666',
            border: 'none',
            padding: '4px 12px',
            borderRadius: '4px',
            cursor: isWritable ? 'pointer' : 'not-allowed',
        };

        return (
            <div
                className="top-bar"
                style={{
                    display: 'flex',
                    justifyContent: 'space-between',
                    alignItems: 'center',
                    padding: '0 15px',
                    backgroundColor: '#1a1a1a',
                    color: '#d2d2d2',
                    fontFamily: 'Inter, system-ui, sans-serif',
                    height: '36px',
                    borderBottom: '1px solid #333',
                    fontSize: '14px',
                }}
            >
                <div className="title" style={{ fontWeight: 600 }}>
                    {title}
                </div>
                <div className="buttons" style={{ display: 'flex', gap: '8px' }}>
                    <button onClick={isWritable ? onUploadClick : undefined} disabled={!isWritable} style={btnStyle}>
                        Upload
                    </button>
                    <button onClick={isWritable ? onDownloadClick : undefined} disabled={!isWritable} style={btnStyle}>
                        Download
                    </button>
                </div>
            </div>
        );
    }
}
