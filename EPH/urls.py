from django.conf.urls.static import static
from django.urls import path
from django.views.generic import TemplateView

from EPH.views import *

app_name = 'EPH'

urlpatterns = [
    path('register/', RegisterView.as_view(), name='register'),

    path('login/', LoginView.as_view(), name='login'),

    path('capture/', CapturePhotoView.as_view(), name='capture'),

    path('getphotos/', PhotoListView.as_view(), name='getphotos'),

    path('objectdetect/', ObjectDetectView.as_view(), name='objectdetect'),

    path('lockdoor/', LockDoorView.as_view(), name='lockdoor'),

    path('opendoor/', OpenDoorView.as_view(), name='opendoor'),

    path('getlockinfo/', GetLockInfoView.as_view(), name='getlockinfo'),
]
urlpatterns += [
    path('', TemplateView.as_view(template_name="index.html"), name="home"),
]
urlpatterns += static("/photos/", document_root=os.path.join(settings.BASE_DIR, "photos"))