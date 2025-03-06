from django.apps import AppConfig

class SubscriptionConfig(AppConfig):
    default_auto_field = 'django.db.models.BigAutoField'
    name = 'subscription'

    def ready(self):
        """ Django 启动时自动运行 MQTT 监听 """
        from . import sub  # 现在 Django 可以找到 sub.py