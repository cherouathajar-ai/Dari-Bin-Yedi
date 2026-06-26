import tkinter as tk
import paho.mqtt.client as mqtt
import urllib.request
import urllib.parse
from datetime import datetime

# 1. Broker Settings
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC_MODE = "dari/control/mode"

# 2. Telegram Bot Settings
TELEGRAM_TOKEN = "8814935222:AAEpgcw97LX7KS-vaX79-gTMyUA-2Uy8shs"
TELEGRAM_CHAT_ID = "2073891106"

def send_telegram_alert(alert_text):
    try:
        safe_text = urllib.parse.quote(alert_text)
        url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendMessage?chat_id={TELEGRAM_CHAT_ID}&text={safe_text}"
        urllib.request.urlopen(url)
        print(">>> Telegram notification sent! <<<")
    except Exception as e:
        print(f"Failed to send Telegram message: {e}")

# 3. Create the MQTT Client
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode("utf-8")
    
    if topic == "dari/alerts":
        alert_label.config(text=f"🚨 ALERT: {payload}", fg="red")
        window.after(5000, lambda: alert_label.config(text="Security: All clear", fg="green"))
    elif topic == "dari/sensors/temp":
        lbl_temp.config(text=f"{payload} °C")
    elif topic == "dari/sensors/hum":
        lbl_hum.config(text=f"{payload} %")
    elif topic == "dari/sensors/gas":
        lbl_gas.config(text=f"{payload} ppm")
    elif topic == "dari/sensors/door":
        # STANDARD LOGIC: Reads exactly what the ESP32 sends
        lbl_door.config(text=f"{payload}", fg="#c62828" if payload == "OPEN" else "#2e7d32")
    elif topic == "dari/sensors/window":
        # STANDARD LOGIC: Reads exactly what the ESP32 sends
        lbl_window.config(text=f"{payload}", fg="#c62828" if payload == "OPEN" else "#2e7d32")
    elif topic == "dari/sensors/rain":
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        lbl_rain.config(text=f"Last Rain: {now}")

client.on_message = on_message
client.connect(BROKER, PORT)
topics_to_subscribe = [
    ("dari/alerts", 0), ("dari/sensors/temp", 0), ("dari/sensors/hum", 0),
    ("dari/sensors/gas", 0), ("dari/sensors/door", 0), ("dari/sensors/window", 0),
    ("dari/sensors/rain", 0)
]
client.subscribe(topics_to_subscribe)
client.loop_start() 

# 4. Control Functions
def set_home():
    client.publish(TOPIC_MODE, "HOME")
    status_label.config(text="Status: Mode set to HOME", fg="#2e7d32")

def set_away():
    client.publish(TOPIC_MODE, "AWAY")
    status_label.config(text="Status: Mode set to AWAY", fg="#c62828")

def toggle_light(room_name, state):
    topic = f"dari/control/lights/{room_name}"
    client.publish(topic, state)
    status_label.config(text=f"Status: {room_name.replace('_', ' ').title()} Lights {state}", fg="#e67e22" if state=="ON" else "#555555")

# --- SIMULATION LOGIC FOR PRESENTATION ---
current_sim_mode = "HOME"

def simulate_rfid(person):
    global current_sim_mode
    # 1. Add Timestamp to Log
    now = datetime.now().strftime("%H:%M:%S")
    log_entry = f"[{now}] Door opened by: {person}\n"
    rfid_log.insert(tk.END, log_entry)
    rfid_log.see(tk.END)
    
    # 2. Toggle the System Mode visually
    if current_sim_mode == "AWAY":
        set_home()
        current_sim_mode = "HOME"
        alert_msg = f"System Disarmed by {person}"
    else:
        set_away()
        current_sim_mode = "AWAY"
        alert_msg = f"System Armed by {person}"
    
    # 3. Update Alert Banner & Send Telegram
    alert_label.config(text=f"✅ {alert_msg}", fg="blue")
    send_telegram_alert(alert_msg)
    window.after(5000, lambda: alert_label.config(text="Security: All clear", fg="green"))

# 5. Draw the Window
window = tk.Tk()
window.title("Dari Bin Yedi - Smart Hub")
window.geometry("450x820") 
window.configure(bg="#f4f4f9")

tk.Label(window, text="🏠 Dari Bin Yedi", font=("Helvetica", 18, "bold"), bg="#f4f4f9").pack(pady=5)

# --- SECURITY MODES ---
security_frame = tk.LabelFrame(window, text="Security Modes", bg="#f4f4f9", font=("Helvetica", 10, "bold"), padx=10, pady=5)
security_frame.pack(fill="x", padx=20, pady=5)
tk.Button(security_frame, text="🏠 Set to HOME", font=("Helvetica", 10, "bold"), bg="#a8e6cf", width=16, command=set_home).pack(side="left", padx=10)
tk.Button(security_frame, text="🔒 Set to AWAY", font=("Helvetica", 10, "bold"), bg="#ff8b94", width=16, command=set_away).pack(side="right", padx=10)

# --- LIVE TELEMETRY ---
telemetry_frame = tk.LabelFrame(window, text="Live Environment", bg="#f4f4f9", font=("Helvetica", 10, "bold"), padx=10, pady=5)
telemetry_frame.pack(fill="x", padx=20, pady=5)
tk.Label(telemetry_frame, text="🌡️ Temperature:", bg="#f4f4f9").grid(row=0, column=0, sticky="w", pady=2)
lbl_temp = tk.Label(telemetry_frame, text="-- °C", bg="#f4f4f9", font=("Helvetica", 10, "bold"))
lbl_temp.grid(row=0, column=1, sticky="w")

tk.Label(telemetry_frame, text="💧 Humidity:", bg="#f4f4f9").grid(row=1, column=0, sticky="w", pady=2)
lbl_hum = tk.Label(telemetry_frame, text="-- %", bg="#f4f4f9", font=("Helvetica", 10, "bold"))
lbl_hum.grid(row=1, column=1, sticky="w")

tk.Label(telemetry_frame, text="💨 Kitchen Gas Lvl:", bg="#f4f4f9").grid(row=2, column=0, sticky="w", pady=2)
lbl_gas = tk.Label(telemetry_frame, text="-- ppm", bg="#f4f4f9", font=("Helvetica", 10, "bold"))
lbl_gas.grid(row=2, column=1, sticky="w")

# --- SENSORS & WEATHER ---
state_frame = tk.LabelFrame(window, text="Doors & Weather", bg="#f4f4f9", font=("Helvetica", 10, "bold"), padx=10, pady=5)
state_frame.pack(fill="x", padx=20, pady=5)
tk.Label(state_frame, text="🚪 Main Door:", bg="#f4f4f9").grid(row=0, column=0, sticky="w", pady=2)
lbl_door = tk.Label(state_frame, text="UNKNOWN", bg="#f4f4f9", font=("Helvetica", 10, "bold"))
lbl_door.grid(row=0, column=1, sticky="w")

tk.Label(state_frame, text="🪟 Window:", bg="#f4f4f9").grid(row=1, column=0, sticky="w", pady=2)
lbl_window = tk.Label(state_frame, text="UNKNOWN", bg="#f4f4f9", font=("Helvetica", 10, "bold"))
lbl_window.grid(row=1, column=1, sticky="w")

lbl_rain = tk.Label(state_frame, text="Last Rain: No data yet", bg="#f4f4f9", font=("Helvetica", 10, "italic"), fg="blue")
lbl_rain.grid(row=2, column=0, columnspan=2, sticky="w", pady=5)

# --- RFID ACCESS LOG (WITH SIMULATION) ---
log_frame = tk.LabelFrame(window, text="RFID Access Log", bg="#f4f4f9", font=("Helvetica", 10, "bold"), padx=10, pady=5)
log_frame.pack(fill="x", padx=20, pady=5)
rfid_log = tk.Text(log_frame, height=4, width=40, font=("Courier", 9), bg="#e8e8e8")
rfid_log.pack(pady=5)

# Simulation Buttons
sim_frame = tk.Frame(log_frame, bg="#f4f4f9")
sim_frame.pack(fill="x", pady=2)
tk.Button(sim_frame, text="Scan Husband RFID", bg="#b3e5fc", command=lambda: simulate_rfid("Husband")).pack(side="left", padx=5, expand=True, fill="x")
tk.Button(sim_frame, text="Scan Wife RFID", bg="#f8bbd0", command=lambda: simulate_rfid("Wife")).pack(side="right", padx=5, expand=True, fill="x")

# --- ROOM CONTROLS ---
rooms_frame = tk.LabelFrame(window, text="Room Controls", bg="#f4f4f9", font=("Helvetica", 10, "bold"), padx=20, pady=5)
rooms_frame.pack(fill="x", padx=20, pady=5)
for i, room in enumerate([("🍳 Kitchen", "kitchen"), ("🛋️ Living Room", "living_room"), ("🛏️ Bedroom", "bedroom")]):
    tk.Label(rooms_frame, text=room[0], font=("Helvetica", 11), bg="#f4f4f9", width=12, anchor="w").grid(row=i, column=0, pady=2)
    tk.Button(rooms_frame, text="ON", bg="#fcf3cf", width=6, command=lambda r=room[1]: toggle_light(r, "ON")).grid(row=i, column=1, padx=5)
    tk.Button(rooms_frame, text="OFF", bg="#d5d8dc", width=6, command=lambda r=room[1]: toggle_light(r, "OFF")).grid(row=i, column=2, padx=5)

# --- STATUS SECTION ---
alert_label = tk.Label(window, text="Security: All clear", font=("Helvetica", 12, "bold"), bg="#f4f4f9", fg="green")
alert_label.pack(pady=5)
status_label = tk.Label(window, text="Status: Connected and Ready", font=("Helvetica", 10, "italic"), bg="#f4f4f9", fg="#555555")
status_label.pack(pady=2)

window.mainloop()
client.loop_stop()