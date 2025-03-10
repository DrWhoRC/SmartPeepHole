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
            return Response({"error": "user name already exists"}, status=400)

        user = UserBasicModel(name=name)
        user.set_password(password)  # 加密密码
        user.save()

        return Response({"message": "register successfully"})

class LoginView(APIView):
    def post(self, request):
        name = request.data.get('name')
        password = request.data.get('password')

        user = UserBasicModel.objects.filter(name=name).first()
        if not user or not user.check_password(password):  # 使用 Django 的 check_password()
            return Response({"error": "name or pwd wrong"}, status=400)

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
        return Response({"message": f"welcome，{request.user.username}，authorization succeed！"})

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
        publish.single("camera/capture", "capture", hostname=MQTT_BROKER)
        return Response({"status": "capture command sent"})


PHOTO_DIR = os.path.join(settings.BASE_DIR, "photos")

class PhotoListView(APIView):
    # permission_classes = [IsAuthenticated]

    def get(self, request):
        """ 直接获取 `photos/` 目录下最新 3 张照片的文件名 """
        if not os.path.exists(PHOTO_DIR):
            return Response({"photos": []})  # 确保目录存在

        # 获取 `photos/` 目录下的所有照片，按修改时间排序
        photos = sorted(
            [p for p in os.listdir(PHOTO_DIR) if p.endswith((".jpg", ".jpeg", ".png"))],
            key=lambda x: os.path.getmtime(os.path.join(PHOTO_DIR, x)),
            reverse=True
        )[:3]  # 取最新 3 张照片

        # 生成可访问的 URL（前端直接访问）
        photo_urls = [f"/photos/{p}" for p in photos]

        return Response({"photos": photo_urls})