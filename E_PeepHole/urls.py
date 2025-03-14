"""
URL configuration for E_PeepHole project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/4.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
import os

from django.conf.urls.static import static
from django.contrib import admin
from django.urls import path, include
from django.views.generic import TemplateView

from E_PeepHole import settings

urlpatterns = [
    path('admin/', admin.site.urls),

    path('user/', include(('EPH.urls', 'EPH'), namespace='EPH')),
]
urlpatterns += [
    path('', TemplateView.as_view(template_name="index.html"), name="home"),
]
urlpatterns += static("/photos/", document_root=os.path.join(settings.BASE_DIR, "photos"))
urlpatterns += static("/static/", document_root=os.path.join(settings.BASE_DIR, "static"))