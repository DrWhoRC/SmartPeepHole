from django.db import models
from django.contrib.auth.hashers import make_password, check_password
# Create your models here.

class UserBasicModel(models.Model):
    id = models.AutoField(primary_key=True)
    name = models.CharField(max_length=30, unique=True, null=False)#no repeat and no null
    password = models.CharField(max_length=128)  # 存储加密后的密码
    created_at = models.DateTimeField(auto_now_add=True)

    def set_password(self, raw_password):
        """ 使用 Django 内置的 `make_password()` 进行加密 """
        self.password = make_password(raw_password)

    def check_password(self, raw_password):
        """ 验证密码 """
        return check_password(raw_password, self.password)

class ObjectDetectModel(models.Model):
    id = models.AutoField(primary_key=True)
    created_at = models.DateTimeField(auto_now_add=True)

class PhotoModel(models.Model):
    id = models.AutoField(primary_key=True)
    created_at = models.DateTimeField(auto_now_add=True)

#python manage.py makemigrations
#python manage.py migrate


