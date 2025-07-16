from ultralytics import YOLO
import cv2

# Load pretrained model
model = YOLO("yolov8n.pt")  # You can also use yolov8s.pt, etc.

# Open the default webcam (0 = default cam)
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Cannot access the webcam.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Run inference on the current frame
    results = model(frame)

    # Plot bounding boxes on the frame
    annotated_frame = results[0].plot()

    # Show the result in a window
    cv2.imshow("YOLOv8 Webcam Detection", annotated_frame)

    # Exit if ESC key is pressed
    if cv2.waitKey(1) & 0xFF == 27:
        break

# Release resources
cap.release()
cv2.destroyAllWindows()
