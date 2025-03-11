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
            // 登录成功后加载所有信息
            loadRecentPhotos();
            loadObjectDetectRecords();
            loadDoorRecords();
        } else {
            document.getElementById("loginStatus").innerText = "login failed";
        }
    })
    .catch(err => console.error("login failed:", err));
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
        setTimeout(loadRecentPhotos, 5000);
    })
    .catch(err => console.error("capture failed:", err));
}

function lockDoor() {
    if (!accessToken) {
        document.getElementById("doorStatus").innerText = "please login first";
        return;
    }

    fetch(BASE_URL + "/user/lockdoor/", {
        method: "POST",
        headers: {
            "Authorization": "Bearer " + accessToken,
            "Content-Type": "application/json"
        }
    })
    .then(res => res.json())
    .then(data => {
        document.getElementById("doorStatus").innerText = `door locked at ${new Date(data.time).toLocaleString()}`;
        loadDoorRecords();
    })
    .catch(err => {
        document.getElementById("doorStatus").innerText = "lock failed, please try again";
        console.error("lock door failed:", err);
    });
}

function openDoor() {
    if (!accessToken) {
        document.getElementById("doorStatus").innerText = "please login first";
        return;
    }

    fetch(BASE_URL + "/user/opendoor/", {
        method: "POST",
        headers: {
            "Authorization": "Bearer " + accessToken,
            "Content-Type": "application/json"
        }
    })
    .then(res => res.json())
    .then(data => {
        document.getElementById("doorStatus").innerText = `door opened at ${new Date(data.time).toLocaleString()}`;
        loadDoorRecords();
    })
    .catch(err => {
        document.getElementById("doorStatus").innerText = "open failed, please try again";
        console.error("open door failed:", err);
    });
}

// 加载最近 3 张照片
function loadRecentPhotos() {
    if (!accessToken) {
        return;
    }

    fetch(BASE_URL + "/user/getphotos/")
    .then(res => res.json())
    .then(data => {
        const gallery = document.getElementById("photoGallery");
        gallery.innerHTML = "";  // 清空旧内容

        data.photos.forEach(photoPath => {
            console.log("load pictures:", BASE_URL + photoPath);
            const img = document.createElement("img");
            img.src = BASE_URL + photoPath;
            img.alt = "pictures";
            gallery.appendChild(img);
        });
    })
    .catch(err => console.error("loading pictures failed:", err));
}

// 加载物体检测记录
function loadObjectDetectRecords() {
    if (!accessToken) {
        return;
    }

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

function loadDoorRecords() {
    if (!accessToken) {
        return;
    }

    fetch(BASE_URL + "/user/getlockinfo/")
    .then(res => res.json())
    .then(data => {
        const list = document.getElementById("doorRecordsList");
        list.innerHTML = "";

        const li = document.createElement("li");
        li.innerText = `${new Date(data.time).toLocaleString()} - Door ${data.msg ? 'locked' : 'opened'}`;
        list.appendChild(li);
    })
    .catch(err => console.error("loading door records failed:", err));
}

