import asyncio
import serial
import serial.tools.list_ports
import websockets
import json
from datetime import datetime
import os

PORT = 'COM3'
BAUD = 9600
JSON_FILE = "users.json"

clients = set()
ser = None
user_list = []

# ----------------------------------------------------
def load_users():
    global user_list
    if os.path.exists(JSON_FILE):
        try:
            with open(JSON_FILE, "r") as f:
                user_list = json.load(f)
                print(f"[OK] Loaded {len(user_list)} users.")
        except:
            user_list = []
    else:
        user_list = []

def save_users():
    with open(JSON_FILE, "w") as f:
        json.dump(user_list, f, indent=2)
    print("[OK] users.json updated.")

def try_connect_serial():
    global ser
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"[OK] Connected to Arduino on {PORT}")
    except Exception as e:
        print(f"[ERROR] Could not open {PORT}: {e}")
        ser = None

def is_serial_connected():
    return ser and ser.is_open

# ----------------------------------------------------
def get_next_id():
    """Finds the smallest available ID_No starting from 1."""
    existing_ids = sorted([u.get("ID_No", 0) for u in user_list])
    next_id = 1
    for eid in existing_ids:
        if eid == next_id:
            next_id += 1
        else:
            break
    return next_id

# ----------------------------------------------------
async def notify_status():
    status = "STATUS:CONNECTED" if is_serial_connected() else "STATUS:DISCONNECTED"
    await asyncio.gather(*(ws.send(status) for ws in clients.copy()))

# ----------------------------------------------------
async def handle_client(websocket):
    clients.add(websocket)
    print("[Client connected]")
    await notify_status()

    async def serial_to_browser():
        while True:
            if ser and ser.in_waiting:
                line = ser.readline().decode(errors='ignore').strip()
                if not line:
                    await asyncio.sleep(0.05)
                    continue

                print(f"[From Arduino] {line}")

                if line.startswith("ENROLL_SUCCESS:"):
                    roll = line.split(":", 1)[1]
                    if not any(u["roll"] == roll for u in user_list):
                        # Assign smallest available ID_No
                        next_id = get_next_id()
                        new_user = {
                            "ID_No": next_id,
                            "roll": roll,
                            "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        }
                        user_list.append(new_user)
                        save_users()
                        await asyncio.gather(*(ws.send("JSON_UPDATED:SUCCESS") for ws in clients.copy()))

                    await asyncio.gather(*(ws.send("USERS:" + json.dumps(user_list)) for ws in clients.copy()))

                elif line.startswith("ENROLL_FAIL_DUPLICATE_ROLL"):
                    await asyncio.gather(*(ws.send("LCD:Roll already exists!") for ws in clients.copy()))

                elif line.startswith("ENROLL_FAIL_DUPLICATE_FINGER"):
                    await asyncio.gather(*(ws.send("LCD:This finger is already enrolled!") for ws in clients.copy()))

                elif line.startswith("DELETE_SUCCESS:"):
                    roll = line.split(":", 1)[1]
                    before_count = len(user_list)
                    user_list[:] = [u for u in user_list if u["roll"] != roll]
                    if len(user_list) != before_count:
                        save_users()
                    await asyncio.gather(*(ws.send("DELETE_UPDATED:SUCCESS") for ws in clients.copy()))
                    await asyncio.gather(*(ws.send("USERS:" + json.dumps(user_list)) for ws in clients.copy()))

                else:
                    await asyncio.gather(*(ws.send(line) for ws in clients.copy()))
            await asyncio.sleep(0.05)

    asyncio.create_task(serial_to_browser())

    try:
        async for msg in websocket:
            print("[From browser]", msg)
            if msg == "GET_STATUS":
                await notify_status()
            elif msg.startswith("ENROLL:"):
                roll = msg.split(":", 1)[1]
                if is_serial_connected():
                    ser.write((f"ENROLL:{roll}\n").encode())
            elif msg == "ENROLL_CANCEL":
                if is_serial_connected():
                    ser.write(b"ENROLL_CANCEL\n")
            elif msg == "LIST_USERS":
                await websocket.send("USERS:" + json.dumps(user_list))
            elif msg.startswith("DELETE:"):
                roll = msg.split(":", 1)[1]
                before_count = len(user_list)
                user_list[:] = [u for u in user_list if u["roll"] != roll]
                if len(user_list) != before_count:
                    save_users()
                await asyncio.gather(*(ws.send("DELETE_UPDATED:SUCCESS") for ws in clients.copy()))
                await asyncio.gather(*(ws.send("USERS:" + json.dumps(user_list)) for ws in clients.copy()))
                if is_serial_connected():
                    ser.write((f"DELETE:{roll}\n").encode())
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        clients.discard(websocket)
        print("[Client disconnected]")

# ----------------------------------------------------
async def main():
    load_users()
    try_connect_serial()
    async with websockets.serve(handle_client, "localhost", 8765):
        print("[Server running at ws://localhost:8765]")
        await asyncio.Future()

if __name__ == "__main__":
    asyncio.run(main())
