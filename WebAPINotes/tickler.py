
import logging
import requests
import time

from config import globals
from .cpwalib import checkResponse
from . import endpoints

def tickle(interval: int = 60):
    """ 
    Background Ticker (keeps session logged in)

    Assumptions:
        (1) Port 5000 open; else connection refused error, would require try-except blocks
    """
    while True: # globals.tickle_running
        requests.get(url=endpoints.base_url+endpoints.tickle, verify=False)
        logging.info(f"{endpoints.tickle} signal sent.")
        # Check validity of response
        req_url: str = endpoints.base_url+endpoints.auth_status
        req_auth = requests.get(url=req_url, verify=False)
        resp: bool = checkResponse(req_url, req_auth)
        status: dict = req_auth.json() if resp else {}
        logging.info(
            f"Authentication status: {status.get('authenticated', '[empty response]')}; "
            f"Connection status: {status.get('connected', '[empty response]')}"
        )
        logging.warning("User is not authenticated") if not status.get('authenticated', False) else None 
        logging.warning("User is not connected") if not status.get('connected', False) else None
        # Sleep until next loop
        time.sleep(interval)

def main():
    logging.basicConfig(
        level=logging.INFO,  # Set log level to INFO
        format='%(asctime)s - %(levelname)s - %(message)s',  # Include timestamp
        handlers=[
            logging.FileHandler(globals.status_file, mode='a')  # Append to the log file
        ],
        force=True
    )
    tickle()

if __name__ == "__main__":
    main()



