from polygon import WebSocketClient
from polygon.websocket.models import WebSocketMessage
from typing import List
import time

API_KEY = 'vaKxLEdcRQssFpe6vHdFpj7VegVZPj8L'

ws = WebSocketClient(api_key=API_KEY, subscriptions=["T.AAPL"])

def handle_msg(msg: List[WebSocketMessage]):
    for m in msg:
        print(m)

ws.run(handle_msg=handle_msg)