import cv2
import os

# 5 object classes
classes = ["Apple", "Mug", "Nutella", "Orange", "Pop"]

dataset_path = "dataset"
if not os.path.exists(dataset_path):
    os.makedirs(dataset_path)

for category in classes:
    category_path = os.path.join(dataset_path, category)
    if not os.path.exists(category_path):
        os.makedirs(category_path)

# Open webcam
cap = cv2.VideoCapture(0)

# resolution to 640x480 (matching Arducame)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

print("Press 'C' to capture an image, 'N' to switch category, and 'Q' to quit.")

current_category_index = 0
image_count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Define bounding box (centered at 320x240)
    x_start, y_start, x_end, y_end = 220, 140, 420, 340
    cv2.rectangle(frame, (x_start, y_start), (x_end, y_end), (0, 255, 0), 2)

    # Display instructions
    text = f"Category: {classes[current_category_index]} | Images: {image_count}"
    cv2.putText(frame, text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.putText(frame, "Place object inside the box", (x_start, y_start - 10), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
    cv2.imshow("Capture Dataset", frame)

    key = cv2.waitKey(1) & 0xFF

    # Capture and crop image with bounding box
    if key == ord('c'):
        cropped_img = frame[y_start:y_end, x_start:x_end]  # Crop bounding box area
        img_path = os.path.join(dataset_path, classes[current_category_index], f"{image_count}.jpg")
        cv2.imwrite(img_path, cropped_img)
        image_count += 1
        print(f"Saved: {img_path}")

    # Switch category
    elif key == ord('n'):
        current_category_index = (current_category_index + 1) % len(classes)
        image_count = 0
        print(f"Switched to: {classes[current_category_index]}")

    # Quit
    elif key == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()