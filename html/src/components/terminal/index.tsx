import { bind } from 'decko';
import { Component, h } from 'preact';
import { Xterm, XtermOptions } from './xterm';
import { TopBar } from '../top-bar';

import '@xterm/xterm/css/xterm.css';
import { Modal } from '../modal';

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

export class Terminal extends Component<Props, State> {
    private container: HTMLElement;
    private xterm: Xterm;

    constructor(props: Props) {
        super();
        this.setState({ title: 'ttya', actionModal: 'none', remotePath: '', writable: true });
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
        // Prevent default form submission if wrapped in form
        e.preventDefault();
        const fileInput = document.getElementById('upload-file-input') as HTMLInputElement;
        const files = fileInput?.files;
        if (!files || files.length === 0 || !this.state.remotePath) {
            alert('Please select a file and enter a path');
            return;
        }

        // We stream the file via POST body to /upload?path=...
        // Fetch API allows body to be file.
        // But implementation in http.c expects raw body write to file.
        // If we use FormData, browser sends multipart/form-data with boundaries.
        // our http.c server writes *everything* receiving to the file.
        // So we must NOT use FormData if our backend logic is just "write body to file".
        // functionality in http.c:
        //    pss->upload_fd = open(path_arg, ...);
        //    write(pss->upload_fd, in, len);
        // This suggests raw body.

        const file = files[0];
        try {
            const resp = await fetch(
                `/upload?path=${encodeURIComponent(this.state.remotePath)}&filename=${encodeURIComponent(file.name)}`,
                {
                    method: 'POST',
                    body: file,
                }
            );
            if (resp.ok) {
                alert('Upload successful');
                this.closeActionModal();
            } else {
                const errorText = await resp.text();
                alert('Upload failed: ' + (errorText || resp.statusText));
            }
        } catch (err) {
            alert('Upload error: ' + err);
        }
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

    render({ id }: Props, { modal, title, actionModal, remotePath }: State) {
        const modalBgStyle = {
            backgroundColor: '#2b2b2b',
            color: '#d2d2d2',
            padding: '20px',
            borderRadius: '8px',
            boxShadow: '0 4px 12px rgba(0,0,0,0.5)',
            minWidth: '320px',
            border: '1px solid #444',
        };
        const inputStyle = {
            width: '100%',
            padding: '8px',
            marginTop: '5px',
            backgroundColor: '#1a1a1a',
            color: '#fff',
            border: '1px solid #444',
            borderRadius: '4px',
        };
        const btnRowStyle = {
            display: 'flex',
            justifyContent: 'flex-end',
            gap: '10px',
            marginTop: '20px',
        };
        const btnPrimaryStyle = {
            backgroundColor: '#427ab3',
            color: '#fff',
            border: 'none',
            padding: '6px 16px',
            borderRadius: '4px',
            cursor: 'pointer',
        };
        const btnSecondaryStyle = {
            backgroundColor: '#555',
            color: '#fff',
            border: 'none',
            padding: '6px 16px',
            borderRadius: '4px',
            cursor: 'pointer',
        };

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
                />
                <div
                    style={{ flex: 1, position: 'relative', overflow: 'hidden' }}
                    ref={c => { this.container = c as HTMLElement; }}
                ></div>

                <Modal show={modal}>
                    <div style={modalBgStyle}>
                        <h3 style={{ marginTop: 0 }}>ZModem File Upload</h3>
                        <label
                            class="file-label"
                            style={{
                                display: 'block',
                                padding: '20px',
                                border: '2px dashed #444',
                                textAlign: 'center',
                                cursor: 'pointer',
                            }}
                        >
                            <input
                                onChange={this.sendFile}
                                class="file-input"
                                type="file"
                                multiple
                                style={{ display: 'none' }}
                            />
                            <span class="file-cta">Choose files…</span>
                        </label>
                        <div style={btnRowStyle}>
                            <button onClick={() => this.setState({ modal: false })} style={btnSecondaryStyle}>
                                Cancel
                            </button>
                        </div>
                    </div>
                </Modal>

                <Modal show={actionModal !== 'none'}>
                    <div style={modalBgStyle}>
                        <h3 style={{ marginTop: 0 }}>{actionModal === 'upload' ? 'Upload File' : 'Download File'}</h3>
                        <div style={{ marginBottom: '15px' }}>
                            <label style={{ fontSize: '13px', color: '#aaa' }}>Absolute Path on Server:</label> <br />
                            <input
                                type="text"
                                value={remotePath}
                                onInput={this.handlePathChange}
                                style={inputStyle}
                                placeholder="/path/to/file"
                            />
                        </div>

                        {actionModal === 'upload' && (
                            <div style={{ marginBottom: '15px' }}>
                                <label
                                    style={{ fontSize: '13px', color: '#aaa', marginBottom: '5px', display: 'block' }}
                                >
                                    Select File:
                                </label>
                                <label
                                    class="file-label"
                                    style={{
                                        display: 'block',
                                        padding: '10px',
                                        border: '1px dashed #444',
                                        textAlign: 'center',
                                        cursor: 'pointer',
                                        backgroundColor: '#1a1a1a',
                                        borderRadius: '4px',
                                    }}
                                >
                                    <input
                                        id="upload-file-input"
                                        class="file-input"
                                        type="file"
                                        style={{ display: 'none' }}
                                        onChange={e => {
                                            const file = (e.target as HTMLInputElement).files?.[0];
                                            const span = (e.target as HTMLElement).nextElementSibling as HTMLElement;
                                            if (file && span) span.innerText = file.name;
                                        }}
                                    />
                                    <span
                                        class="file-cta"
                                        style={{
                                            border: 'none',
                                            backgroundColor: 'transparent',
                                            padding: 0,
                                            height: 'auto',
                                            color: '#fff',
                                        }}
                                    >
                                        Choose file...
                                    </span>
                                </label>
                            </div>
                        )}

                        <div style={btnRowStyle}>
                            <button onClick={this.closeActionModal} style={btnSecondaryStyle}>
                                Cancel
                            </button>
                            {actionModal === 'upload' ? (
                                <button onClick={this.performUpload} style={btnPrimaryStyle}>
                                    Upload
                                </button>
                            ) : (
                                <button onClick={this.performDownload} style={btnPrimaryStyle}>
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
