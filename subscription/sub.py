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

# 照片存储目录（项目根目录下的 "photos" 文件夹）
PHOTO_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "photos")

# 确保目录存在
os.makedirs(PHOTO_DIR, exist_ok=True)
buffer = bytearray()
receiving = False

def on_message(client, userdata, msg):
    global buffer, receiving

    topic = msg.topic  # 获取主题
    payload = msg.payload.decode(errors="ignore")

    # 处理 "object/detect" 主题
    if topic == TOPIC_OBJECT_DETECT:
        print(f"🔔 物体检测: {payload}")
        ObjectDetectModel.objects.create()  # 存入数据库
        return

    # 处理 "camera/image" 主题
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
            print(f"📸 照片保存至: {filepath}")
            buffer = bytearray()
            receiving = False
            return

        if receiving:
            buffer.extend(msg.payload)  # 追加数据

# 配置 MQTT 客户端
client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# 订阅多个主题
client.subscribe(TOPIC_OBJECT_DETECT)
client.subscribe(TOPIC_IMAGE)

client.loop_start()  # 以后台线程运行
