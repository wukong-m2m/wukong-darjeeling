from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import time

myMQTTClient = AWSIoTMQTTClient("")
myMQTTClient.configureEndpoint("a1trumz0n7avwt.iot.us-west-2.amazonaws.com", 8883)
myMQTTClient.configureCredentials("root.crt", "private.key", "cert.crt")
myMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
myMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
myMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
myMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec

myMQTTClient.connect()

def customCallback(client, userdata, message):
    print("Received a new message: ")
    print(message.payload)
    print("from topic: ")
    print(message.topic)
    print("--------------\n\n")

myMQTTClient.subscribe("WKtopic", 1, customCallback)

while True:
    time.sleep(1)
