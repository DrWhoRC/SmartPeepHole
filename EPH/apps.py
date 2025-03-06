import os
import sys

from django.apps import AppConfig


class EphConfig(AppConfig):
    default_auto_field = 'django.db.models.BigAutoField'
    name = 'EPH'

    def ready(self):
        """ Django 启动时自动运行 MQTT 监听 """
        sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)), "subscription"))
        from subscription import sub  # 确保 Django 启动时加载 MQTT 订阅