from django.urls import path

from EPH.views import *

app_name = 'EPH'

urlpatterns = [
    path('register/', RegisterView.as_view(), name='register'),

    path('login/', LoginView.as_view(), name='login'),

    path('capture/', CapturePhotoView.as_view(), name='capture'),

    path('getphotos/', PhotoListView.as_view(), name='getphotos'),

    path('objectdetect/', ObjectDetectView.as_view(), name='objectdetect'),
]