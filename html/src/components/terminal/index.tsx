import { bind } from 'decko';
import { Component, h } from 'preact';
import { Xterm, XtermOptions } from './xterm';
import { TopBar } from '../top-bar';

import '@xterm/xterm/css/xterm.css';
import { Modal } from '../modal';

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

export class Terminal extends Component<Props, State> {
    private container: HTMLElement;
    private xterm: Xterm;

    constructor(props: Props) {
        super();
        this.setState({ title: 'ttya', actionModal: 'none', remotePath: '', writable: true, uploads: [] });
        this.xterm = new Xterm(props, this.showModal, this.onTitleChange, this.onWritableChange);
    }

    async componentDidMount() {
        await this.xterm.refreshToken();
        this.xterm.open(this.container);
        this.xterm.connect();
    }

    componentWillUnmount() {
        this.xterm.dispose();
    }

    @bind
    onTitleChange(title: string) {
        this.setState({ title });
    }

    @bind
    onWritableChange(writable: boolean) {
        this.setState({ writable });
    }

    @bind
    handleUploadClick() {
        this.setState({ actionModal: 'upload', remotePath: '' });
    }

    @bind
    handleDownloadClick() {
        this.setState({ actionModal: 'download', remotePath: '' });
    }

    @bind
    closeActionModal() {
        this.setState({ actionModal: 'none' });
    }

    @bind
    handlePathChange(e: Event) {
        this.setState({ remotePath: (e.target as HTMLInputElement).value });
    }

    @bind
    async performUpload(e: Event) {
        e.preventDefault();
        const fileInput = document.getElementById('upload-file-input') as HTMLInputElement;
        const files = fileInput?.files;
        if (!files || files.length === 0 || !this.state.remotePath) {
            alert('Please select a file and enter a path');
            return;
        }

        const remotePathBase = this.state.remotePath;
        this.closeActionModal();

        const fileList = Array.from(files);
        const newUploads = fileList.map((file, idx) => ({
            id: `${Date.now()}-${idx}`,
            filename: file.name,
            loaded: 0,
            total: file.size,
            status: 'pending' as const,
        }));

        this.setState(prevState => ({
            uploads: [...(prevState.uploads || []), ...newUploads]
        }));

        for (let i = 0; i < fileList.length; i++) {
            const file = fileList[i];
            const uploadId = newUploads[i].id;
            this.uploadSingleFile(file, remotePathBase, uploadId);
        }
    }

    uploadSingleFile(file: File, remotePathBase: string, uploadId: string) {
        this.updateUploadStatus(uploadId, { status: 'uploading' });

        const xhr = new XMLHttpRequest();
        let targetPath = remotePathBase;
        
        if (targetPath.endsWith('/')) {
            targetPath = targetPath + file.name;
        } else {
            const fileInput = document.getElementById('upload-file-input') as HTMLInputElement;
            const filesCount = fileInput?.files?.length || 1;
            if (filesCount > 1) {
                targetPath = targetPath + '/' + file.name;
            }
        }

        xhr.upload.addEventListener('progress', (e) => {
            if (e.lengthComputable) {
                this.updateUploadStatus(uploadId, {
                    loaded: e.loaded,
                    total: e.total,
                });
            }
        });

        xhr.addEventListener('load', () => {
            if (xhr.status >= 200 && xhr.status < 300) {
                this.updateUploadStatus(uploadId, { status: 'success', loaded: file.size });
                setTimeout(() => {
                    this.clearUpload(uploadId);
                }, 5000);
            } else {
                let errMsg = `${xhr.status} ${xhr.statusText || 'Upload failed'}`;
                try {
                    const json = JSON.parse(xhr.responseText);
                    if (json && json.error) {
                        errMsg = json.error;
                    }
                } catch (e) {}
                this.updateUploadStatus(uploadId, { status: 'error', errorMsg: errMsg });
            }
        });

        xhr.addEventListener('error', () => {
            this.updateUploadStatus(uploadId, { status: 'error', errorMsg: 'Network error (Connection Reset)' });
        });

        xhr.open('POST', `/upload?path=${encodeURIComponent(targetPath)}&filename=${encodeURIComponent(file.name)}`);
        xhr.send(file);
    }

    updateUploadStatus(id: string, updates: Partial<{ loaded: number; total: number; status: 'pending' | 'uploading' | 'success' | 'error'; errorMsg: string }>) {
        this.setState(prevState => {
            const uploads = (prevState.uploads || []).map(u => {
                if (u.id === id) {
                    return { ...u, ...updates };
                }
                return u;
            });
            return { uploads };
        });
    }

    clearUpload(id: string) {
        this.setState(prevState => {
            const uploads = (prevState.uploads || []).filter(u => u.id !== id);
            return { uploads };
        });
    }

    @bind
    async performDownload() {
        if (!this.state.remotePath) {
            alert('Please enter a path');
            return;
        }
        try {
            const resp = await fetch(`/download_token?path=${encodeURIComponent(this.state.remotePath)}`);
            if (resp.ok) {
                const json = await resp.json();
                if (json.token) {
                    // Optimized download: Create dynamic link with download attribute or just use a dummy anchor
                    // Triggering a download via anchor with download attribute usually avoids the "Leave site?" prompt
                    // as it's handled as a background download task.
                    const link = document.createElement('a');
                    link.href = `/download/${json.token}`;
                    link.download = ''; // Filename can't be set reliably here due to server headers, but this helps browser recognize it's a download
                    document.body.appendChild(link);
                    link.click();
                    document.body.removeChild(link);
                    this.closeActionModal();
                } else {
                    alert('Failed to get token');
                }
            } else {
                const errorMsg = await resp.text();
                alert('Download request failed: ' + (errorMsg || resp.statusText));
            }
        } catch (err) {
            alert('Download error: ' + err);
        }
    }

    render({ id }: Props, { modal, title, actionModal, remotePath, uploads }: State) {
        return (
            <div
                id={id}
                style={{
                    display: 'flex',
                    flexDirection: 'column',
                    height: '100%',
                    width: '100%',
                    backgroundColor: '#2b2b2b',
                }}
            >
                <TopBar
                    title={title}
                    onUploadClick={this.handleUploadClick}
                    onDownloadClick={this.handleDownloadClick}
                    writable={this.state.writable}
                    uploads={uploads}
                />
                <div
                    style={{ flex: 1, position: 'relative', overflow: 'hidden' }}
                    ref={c => { this.container = c as HTMLElement; }}
                ></div>

                <Modal show={modal}>
                    <div class="flat-modal-card">
                        <h3>
                            <svg class="dropzone-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style={{ marginBottom: 0 }}><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
                            ZModem File Upload
                        </h3>
                        <label class="flat-dropzone">
                            <input
                                onChange={this.sendFile}
                                class="file-input"
                                type="file"
                                multiple
                            />
                            <svg class="dropzone-icon" width="36" height="36" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
                            <span class="file-cta">Choose files…</span>
                        </label>
                        <div class="btn-row">
                            <button onClick={() => this.setState({ modal: false })} class="btn-flat-secondary">
                                Cancel
                            </button>
                        </div>
                    </div>
                </Modal>

                <Modal show={actionModal !== 'none'}>
                    <div class="flat-modal-card">
                        <h3>
                            {actionModal === 'upload' ? (
                                <svg class="dropzone-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style={{ marginBottom: 0 }}><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
                            ) : (
                                <svg class="dropzone-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style={{ marginBottom: 0 }}><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>
                            )}
                            {actionModal === 'upload' ? 'Upload File' : 'Download File'}
                        </h3>
                        <div style={{ marginBottom: '16px' }}>
                            <label class="field-label">Absolute Path on Server:</label>
                            <input
                                type="text"
                                value={remotePath}
                                onInput={this.handlePathChange}
                                class="flat-input"
                                placeholder="/path/to/file"
                            />
                            <span class="field-desc">Specify the exact path on the remote system</span>
                        </div>

                        {actionModal === 'upload' && (
                            <div style={{ marginBottom: '16px' }}>
                                <label class="field-label">Select File:</label>
                                <label class="flat-dropzone" style={{ padding: '16px 20px' }}>
                                    <input
                                        id="upload-file-input"
                                        type="file"
                                        class="file-input"
                                        multiple
                                        onChange={e => {
                                            const files = (e.target as HTMLInputElement).files;
                                            const label = (e.target as HTMLElement).parentElement;
                                            const span = label?.querySelector('.file-cta') as HTMLElement;
                                            if (files && files.length > 0 && span) {
                                                if (files.length === 1) {
                                                    span.innerText = files[0].name;
                                                } else {
                                                    span.innerText = `${files.length} files selected`;
                                                }
                                            }
                                        }}
                                    />
                                    <svg class="dropzone-icon" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" style={{ marginBottom: '4px' }}><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
                                    <span class="file-cta" style={{ fontSize: '13px' }}>Choose file...</span>
                                </label>
                            </div>
                        )}

                        <div class="btn-row">
                            <button onClick={this.closeActionModal} class="btn-flat-secondary">
                                Cancel
                            </button>
                            {actionModal === 'upload' ? (
                                <button onClick={this.performUpload} class="btn-flat-primary">
                                    Upload
                                </button>
                            ) : (
                                <button onClick={this.performDownload} class="btn-flat-primary">
                                    Download
                                </button>
                            )}
                        </div>
                    </div>
                </Modal>
            </div>
        );
    }

    @bind
    showModal() {
        this.setState({ modal: true });
    }

    @bind
    sendFile(event: Event) {
        this.setState({ modal: false });
        const files = (event.target as HTMLInputElement).files;
        if (files) this.xterm.sendFile(files);
    }
}
