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

export class TopBar extends Component<Props, State> {
    constructor() {
        super();
        this.setState({ showDropdown: false });
    }

    render({ title, onUploadClick, onDownloadClick, writable, uploads = [] }: Props, { showDropdown }: State) {
        const isWritable = writable !== false; // default true if undefined
        const btnStyle = {
            backgroundColor: isWritable ? '#333' : '#2a2a2a',
            color: isWritable ? '#fff' : '#666',
            border: 'none',
            padding: '4px 12px',
            borderRadius: '4px',
            cursor: isWritable ? 'pointer' : 'not-allowed',
        };

        const activeUploads = uploads.filter(u => u.status === 'uploading' || u.status === 'pending');
        
        let totalLoaded = 0;
        let totalSize = 0;
        uploads.forEach(u => {
            totalLoaded += u.loaded;
            totalSize += u.total;
        });
        const percent = totalSize > 0 ? Math.round((totalLoaded / totalSize) * 100) : 0;

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
                    position: 'relative',
                }}
            >
                <div className="title" style={{ fontWeight: 600 }}>
                    {title}
                </div>
                <div className="buttons" style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
                    {uploads.length > 0 && (
                        <div 
                            className="upload-progress-container"
                            onClick={() => this.setState({ showDropdown: !showDropdown })}
                        >
                            <div className="upload-info">
                                <span className="upload-filename">
                                    {activeUploads.length === 1 
                                        ? activeUploads[0].filename 
                                        : activeUploads.length > 1 
                                            ? `Uploading ${activeUploads.length} files` 
                                            : uploads.some(u => u.status === 'error')
                                                ? 'Upload failed'
                                                : 'Upload complete'}
                                </span>
                                <div className="progress-track">
                                    <div className="progress-fill" style={{ width: `${percent}%` }}></div>
                                </div>
                            </div>
                            <div className="upload-dropdown-toggle">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
                                    {showDropdown ? <polyline points="18 15 12 9 6 15"/> : <polyline points="6 9 12 15 18 9"/>}
                                </svg>
                            </div>
                            
                            {showDropdown && (
                                <div className="upload-dropdown-menu" onClick={e => e.stopPropagation()}>
                                    {uploads.map(u => {
                                        const p = u.total > 0 ? Math.round((u.loaded / u.total) * 100) : 0;
                                        return (
                                            <div className={`upload-dropdown-item ${u.status}`}>
                                                <span className="item-name" title={u.filename}>{u.filename}</span>
                                                <div className="item-status">
                                                    <span className="item-progress-text">
                                                        {u.status === 'success' ? 'Done' : u.status === 'error' ? `Failed: ${u.errorMsg || 'Error'}` : `${p}%`}
                                                    </span>
                                                    {u.status === 'uploading' && (
                                                        <svg className="spin" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
                                                            <line x1="12" y1="2" x2="12" y2="6"/><line x1="12" y1="18" x2="12" y2="22"/><line x1="4.93" y1="4.93" x2="7.76" y2="7.76"/><line x1="16.24" y1="16.24" x2="19.07" y2="19.07"/><line x1="2" y1="12" x2="6" y2="12"/><line x1="18" y1="12" x2="22" y2="12"/><line x1="4.93" y1="19.07" x2="7.76" y2="16.24"/><line x1="16.24" y1="8.76" x2="19.07" y2="5.93"/>
                                                        </svg>
                                                    )}
                                                    {u.status === 'success' && (
                                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="#10b981" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
                                                            <polyline points="20 6 9 17 4 12"/>
                                                        </svg>
                                                    )}
                                                    {u.status === 'error' && (
                                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="#ef4444" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
                                                            <line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/>
                                                        </svg>
                                                    )}
                                                </div>
                                            </div>
                                        );
                                    })}
                                </div>
                            )}
                        </div>
                    )}
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
