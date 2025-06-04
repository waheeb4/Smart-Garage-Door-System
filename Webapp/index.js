const mqttHost = "broker.hivemq.com";
const mqttPort = 8000;
const clientId = "web_" + Math.random().toString(16).substring(2, 10);

const client = new Paho.Client(mqttHost, mqttPort, clientId);


client.onConnectionLost = function (responseObject) {
    console.log("MQTT connection lost:", responseObject.errorMessage);
};

client.onMessageArrived = function (message) {
    if (message.destinationName === "garage/sensors/buzzer") {
        if (message.payloadString === "1") {
            document.getElementById("haramyLabel").innerText = "Haramy";
            const notification = document.createElement("div");
            notification.innerText = "🚨Alert: Haramy!🚨";
            notification.style.position = "fixed";
            notification.style.top = "20px";
            notification.style.right = "20px";
            notification.style.background = "#EEE9DD";
            notification.style.color = "red";
            notification.style.padding = "16px 24px";
            notification.style.borderRadius = "8px";
            notification.style.boxShadow = "0 2px 8px rgba(0,0,0,0.2)";
            notification.style.zIndex = "1000";
            notification.style.fontSize = "18px";
            document.body.appendChild(notification);

            setTimeout(() => {
                notification.remove();
            }, 5000);
        } else {
            document.getElementById("haramyLabel").innerText = "No Haramy";
        }
    }

};


document.getElementById("doorToggle").disabled = true;

client.connect({
    onSuccess: function () {
        console.log("MQTT connected");
        client.subscribe("garage/sensors/buzzer");

        document.getElementById("doorToggle").disabled = false;
        client.subscribe("garage/control/door");

        msg = new Paho.Message("close");
        msg.destinationName = "garage/control/door";
        client.send(msg);
    },
    onFailure: function (err) {
        console.log("MQTT connection failed:", err);
    },
    keepAliveInterval: 60,
    useSSL: false
});

document.getElementById("doorToggle").addEventListener("change", function () {
    const msgText = this.checked ? "open" : "close";
    console.log("Publishing to garage/control/door:", msgText);
    const message = new Paho.Message(msgText);
    message.destinationName = "garage/control/door";
    client.send(message);
});