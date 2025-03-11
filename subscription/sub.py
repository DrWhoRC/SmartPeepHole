import os
# os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'E_PeepHole.settings')
# import django
# django.setup()
import paho.mqtt.client as mqtt
from django.core.files.base import ContentFile
from django.utils.timezone import now
from EPH.models import ObjectDetectModel, PhotoModel

MQTT_BROKER = "172.20.10.2"
MQTT_PORT = 1883
TOPIC_OBJECT_DETECT = "object/detect"
TOPIC_IMAGE = "camera/image"

PHOTO_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "photos")

os.makedirs(PHOTO_DIR, exist_ok=True)
buffer = bytearray()
receiving = False

def on_message(client, userdata, msg):
    global buffer, receiving

    topic = msg.topic  # 获取主题
    payload = msg.payload.decode(errors="ignore")

    if topic == TOPIC_OBJECT_DETECT:
        print(f"object detect: {payload}")
        ObjectDetectModel.objects.create()
        return

    elif topic == TOPIC_IMAGE:
        if payload == "START":
            buffer = bytearray()
            receiving = True
            return
        elif payload == "END":
            filename = f"received_{now().strftime('%Y%m%d%H%M%S')}.jpeg"
            filepath = os.path.join(PHOTO_DIR, filename)

            with open(filepath, "wb") as f:
                f.write(buffer)
            PhotoModel.objects.create()
            print(f"photos stored in: {filepath}")
            buffer = bytearray()
            receiving = False
            return

        if receiving:
            buffer.extend(msg.payload)

client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

client.subscribe(TOPIC_OBJECT_DETECT)
client.subscribe(TOPIC_IMAGE)

client.loop_start()
