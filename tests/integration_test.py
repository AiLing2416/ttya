#!/usr/bin/env python3
import os
import subprocess
import time
import urllib.request
import urllib.error
import shutil
import sys

def main():
    print("=== Running Integration Test for Symlink Upload Bypass ===")
    
    # 1. Setup workspace and target paths
    test_dir = "/tmp/ttya_test_ws"
    escaped_file = "/tmp/escaped_file.txt"
    symlink_path = os.path.join(test_dir, "broken_link")
    
    # Clean up any leftover files
    if os.path.exists(test_dir):
        shutil.rmtree(test_dir)
    os.makedirs(test_dir, exist_ok=True)
    
    if os.path.exists(escaped_file):
        os.remove(escaped_file)
        
    # Create the broken symlink pointing outside the workspace to a file that does not exist
    os.symlink(escaped_file, symlink_path)
    
    # 2. Start ttya server in background
    # We use a random high port to avoid conflicts
    port = "17681"
    server_cmd = [
        "/workspace/build/ttya",
        "-p", port,
        "-w", test_dir,
        "-W",
        "sleep", "100"
    ]
    
    print(f"Starting ttya on port {port}...")
    proc = subprocess.Popen(server_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    # Wait for the server to start
    time.sleep(1)
    
    # Check if process is still running
    if proc.poll() is not None:
        print("FAIL: Failed to start ttya server.")
        stdout, stderr = proc.communicate()
        print("stdout:", stdout.decode())
        print("stderr:", stderr.decode())
        sys.exit(1)
        
    test_passed = False
    try:
        # 3. Attempt to upload a file through the broken symlink
        url = f"http://localhost:{port}/upload?path={symlink_path}"
        print(f"Sending POST request to {url}...")
        req = urllib.request.Request(url, data=b"pwned", method="POST")
        
        try:
            with urllib.request.urlopen(req) as response:
                status_code = response.status
                print(f"Response status code: {status_code}")
        except urllib.error.HTTPError as e:
            status_code = e.code
            print(f"HTTPError status code: {status_code}")
            
        # 4. Verify that the upload was rejected (403 Forbidden)
        # and the escaped file was NOT created
        file_created = os.path.exists(escaped_file)
        
        print(f"Escaped file created: {file_created}")
        
        if status_code == 403 and not file_created:
            print("PASS: Symlink upload was rejected and no file was written outside the sandbox.")
            test_passed = True
        elif file_created:
            print("FAIL: Sandbox escaped! File was written outside the workspace.")
        else:
            print(f"FAIL: Unexpected status code {status_code} received.")
            
    finally:
        # 5. Clean up
        print("Stopping ttya server...")
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            
        if os.path.exists(test_dir):
            shutil.rmtree(test_dir)
        if os.path.exists(escaped_file):
            os.remove(escaped_file)
            
    if test_passed:
        print("=== Test PASSED ===")
        sys.exit(0)
    else:
        print("=== Test FAILED ===")
        sys.exit(1)

if __name__ == "__main__":
    main()
