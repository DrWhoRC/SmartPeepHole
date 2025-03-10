const BASE_URL = window.location.origin;  // 自动获取 API 地址
let accessToken = "";

// 登录
function login() {
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    fetch(BASE_URL + "/user/login/", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ name: username, password: password })
    })
    .then(res => res.json())
    .then(data => {
        if (data.access) {
            accessToken = data.access;
            document.getElementById("loginStatus").innerText = `login success, welcome ${data.user}`;
        } else {
            document.getElementById("loginStatus").innerText = "login failed";
        }
    })
    .catch(err => console.error("login failes:", err));
}

// 发送拍照请求
function capturePhoto() {
    if (!accessToken) {
        document.getElementById("captureStatus").innerText = "please login first";
        return;
    }

    fetch(BASE_URL + "/user/capture/", {
        method: "POST",
        headers: {
            "Authorization": "Bearer " + accessToken,
            "Content-Type": "application/json"
        }
    })
    .then(res => res.json())
    .then(data => {
        document.getElementById("captureStatus").innerText = "capture success";
        setTimeout(loadRecentPhotos, 5000);  // 5 秒后刷新照片
    })
    .catch(err => console.error("capture failes:", err));
}

// 加载最近 3 张照片
function loadRecentPhotos() {
    fetch(BASE_URL + "/user/getphotos/")
    .then(res => res.json())
    .then(data => {
        const gallery = document.getElementById("photoGallery");
        gallery.innerHTML = "";  // 清空旧内容

        data.photos.forEach(photoPath => {
            console.log("load pictures:", BASE_URL + photoPath);  // ✅ 调试 URL
            const img = document.createElement("img");
            img.src = BASE_URL + photoPath;  // 直接使用 API 返回的路径
            img.alt = "pictures";
            gallery.appendChild(img);
        });
    })
    .catch(err => console.error("loading pictures failed:", err));
}

// 加载物体检测记录
function loadObjectDetectRecords() {
    fetch(BASE_URL + "/user/objectdetect/")
    .then(res => res.json())
    .then(data => {
        const list = document.getElementById("objectDetectList");
        list.innerHTML = "";  // 清空旧内容

        data.forEach(item => {
            const li = document.createElement("li");
            li.innerText = `${item.time} - ${item.message}`;
            list.appendChild(li);
        });
    })
    .catch(err => console.error("loading detection records failed:", err));
}

// 页面加载时自动刷新
window.onload = function() {
    loadRecentPhotos();
    loadObjectDetectRecords();
};
