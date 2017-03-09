export var date = function () {
    var today = new Date();
    var dd = today.getDate();
    var mm = today.getMonth() + 1;
    var yyyy = today.getFullYear();

    dd = dd < 10 ? "0" + dd: dd;
    mm = mm < 10 ? "0" + mm: mm;

    return (dd + '/' + mm + '/' + yyyy);
}

export var time = function () {
    var today = new Date();
    var hh = today.getHours();
    var mm = today.getMinutes();
    var ss = today.getSeconds();

    return ((((hh == 0 ? 24 : hh) - 1) % 12 + 1) + ":" +
            (mm < 10 ? "0" + mm: mm) + ":" +
            (ss < 10 ? "0" + ss: ss) + " " +
            ((hh == 0 || hh > 12) ? "PM" : "AM"));
}

export var consoleLog = function (x) {
    document.getElementById('console').innerHTML = x;
}

export var consoleError = function (x) {
    document.getElementById('console').innerHTML = "<span class='red'>" + x +"</span>"
}

export var listMediaDevices = function () {
    if (!navigator.mediaDevices || !navigator.mediaDevices.enumerateDevices) {
        console.log("enumerateDevices() not supported.");
        return;
    }
    // List cameras and microphones.
    navigator.mediaDevices.enumerateDevices().then(function(devices) {
        devices.forEach(function(device) {
            //console.log(device.kind + ": " + device.label + " id = " + device.deviceId);
            if(device.kind == "videoinput") {
                document.getElementById('camera-input').innerHTML = document.getElementById('camera-input').innerHTML + "<option>" + device.label + "</option>";
            }
        });
    })
    
    .catch(function(err) {
        console.log(err.name + ": " + err.message);
    });
}



