# ABhi Patel
#Inderpreet Singh
# app.py - main file on google cloud that loads our obhect detection model and posts an output

from flask import Flask, request, jsonify
import tensorflow as tf
import numpy as np
import cv2
import os

app = Flask(__name__)

# Load your trained model
model = tf.keras.models.load_model("object_detector.h5")
classes = ["Apple", "Mug", "Nutella", "Orange", "Pop"] # 5 classes to be detected from

@app.route("/predict", methods=["POST"])
def predict():
    if "image" not in request.files:
        return jsonify({"error": "No image provided"}), 400 #error if no file posted

    file = request.files["image"]
    image = cv2.imdecode(np.frombuffer(file.read(), np.uint8), cv2.IMREAD_COLOR)
    if image is None:
        return jsonify({"error": "Invalid image"}), 400

    image = cv2.resize(image, (224, 224)) / 255.0 #normalise image to suit according to trained model
    image = np.expand_dims(image, axis=0)

    predictions = model.predict(image)
    predicted_class = np.argmax(predictions)
    predicted_label = classes[predicted_class]

    return jsonify({"result": predicted_label})

# Required for Cloud Run to know how to start the app
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 8080)))
