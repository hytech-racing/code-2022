const mqtt = require("mqtt")
const client = mqtt.connect("mqtts://iot.eclipse.org:8883")

let connected = false

client.on("connect", () => {
    client.subscribe("hytech/connected")
    console.log("Subscribed to hytech/connected")
})

client.on("message", (topic, message) => {
    if (topic === "hytech/connected") {
        connected = (message.toString() === "true")
        console.log("received connected true")
        client.subscribe("hytech/message")
    }
    else if (topic === "hytech/message") {
        console.log("Received message %s", message)
    }
})