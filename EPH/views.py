import os

from django.shortcuts import render
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework_simplejwt.tokens import RefreshToken
from rest_framework_simplejwt.views import TokenObtainPairView

from E_PeepHole import settings
from .models import UserBasicModel, ObjectDetectModel
from rest_framework.permissions import IsAuthenticated
import paho.mqtt.publish as publish
# Create your views here.
MQTT_BROKER = "172.20.10.2"
class RegisterView(APIView):
    def post(self, request):
        name = request.data.get('name')
        password = request.data.get('password')

        if UserBasicModel.objects.filter(name=name).exists():
            return Response({"error": "用户名已存在"}, status=400)

        user = UserBasicModel(name=name)
        user.set_password(password)  # 加密密码
        user.save()

        return Response({"message": "✅ 用户注册成功"})

class LoginView(APIView):
    def post(self, request):
        name = request.data.get('name')
        password = request.data.get('password')

        user = UserBasicModel.objects.filter(name=name).first()
        if not user or not user.check_password(password):  # 使用 Django 的 check_password()
            return Response({"error": "用户名或密码错误"}, status=400)

        # 生成 JWT Token
        refresh = RefreshToken.for_user(user)
        return Response({
            "refresh": str(refresh),
            "access": str(refresh.access_token),
            "user": user.name
        })



class ProtectedView(APIView):
    permission_classes = [IsAuthenticated]

    def get(self, request):
        return Response({"message": f"欢迎，{request.user.username}，您已通过认证！"})

class ObjectDetectView(APIView):
    # permission_classes = [IsAuthenticated]

    def get(self, request):
        records = ObjectDetectModel.objects.order_by('-created_at')[:10]
        data = [{"time": r.created_at, "message": "object detected"} for r in records]
        return Response(data)

class CapturePhotoView(APIView):
    # permission_classes = [IsAuthenticated]

    def post(self, request):
        """ 发送 MQTT 消息触发 ESP32-CAM 拍照 """
        publish.single("mqtt/control/capture", "capture", hostname=MQTT_BROKER)
        return Response({"status": "📸 拍照指令已发送"})

PHOTO_DIR = os.path.join(settings.BASE_DIR, "photos")

class PhotoListView(APIView):
    # permission_classes = [IsAuthenticated]

    def get(self, request):
        """ 获取 `photos/` 文件夹下的所有照片 """
        photos = sorted(os.listdir(PHOTO_DIR), reverse=True)[:10]  # 取最新10张照片
        photo_urls = [request.build_absolute_uri(f"/photos/{p}") for p in photos]
        return Response({"photos": photo_urls})