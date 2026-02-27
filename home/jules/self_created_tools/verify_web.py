import os
import time
import subprocess
import requests
from playwright.sync_api import sync_playwright

def verify_web_interface():
    # Start ttya server in background
    port = 8085
    cmd = ["./build/ttya", "-p", str(port), "-W", "bash"]
    print(f"Starting ttya on port {port}")
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(2)  # Wait for server to start

    try:
        with sync_playwright() as p:
            browser = p.chromium.launch()
            page = browser.new_page()
            url = f"http://localhost:{port}"
            print(f"Navigating to {url}")
            page.goto(url)

            # Wait for terminal to load (check for xterm container)
            print("Waiting for terminal to load...")
            page.wait_for_selector(".xterm-screen")

            # Take a screenshot
            screenshot_path = "ttya_web_verify.png"
            page.screenshot(path=screenshot_path)
            print(f"Screenshot saved to {screenshot_path}")

            # Basic verification: Check title
            title = page.title()
            print(f"Page title: {title}")
            if "ttya" in title:
                print("SUCCESS: Web interface loaded and title correct.")
            else:
                print("FAILURE: Web interface title mismatch.")

            browser.close()

    except Exception as e:
        print(f"Verification failed: {e}")
    finally:
        process.terminate()
        process.wait()

if __name__ == "__main__":
    verify_web_interface()
