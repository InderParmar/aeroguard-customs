import os
from ultralytics import YOLO
import cv2
from pathlib import Path

# === CONFIGURATION ===
MODEL_PATH = "newbestJune6.pt"  # Path to trained YOLO model
TEST_DIR = "test_samples2/"                                  # Folder containing test images
OUTPUT_DIR = "inference_results/"                           # Folder to save annotated images
CONFIDENCE_THRESHOLD = 0.15                                 # Set confidence threshold

# === PREPARE OUTPUT FOLDER ===
os.makedirs(OUTPUT_DIR, exist_ok=True)

# === LOAD YOLO MODEL ===
model = YOLO(MODEL_PATH)

# === INFERENCE ON ALL IMAGES IN FOLDER ===
image_extensions = ['.jpg', '.jpeg', '.png', '.gif']
image_paths = [os.path.join(TEST_DIR, fname) for fname in os.listdir(TEST_DIR)
               if Path(fname).suffix.lower() in image_extensions]

print(f"\nFound {len(image_paths)} images to test...")

for img_path in image_paths:
    results = model.predict(
        source=img_path,
        conf=CONFIDENCE_THRESHOLD,
        save=True,
        save_txt=False,
        save_crop=False,
        save_conf=True,
        project=OUTPUT_DIR,
        name="predictions",   # All results will go to inference_results/predictions
        exist_ok=True
    )
    
    # Print prediction summary
    print(f"Image: {img_path}")
    for result in results:
        boxes = result.boxes
        for i, box in enumerate(boxes):
            cls = int(box.cls[0])
            conf = float(box.conf[0])
            print(f" - Object {i+1}: Class={model.names[cls]}, Confidence={conf:.2f}")

print(f"\nDone. Annotated images saved to: {os.path.join(OUTPUT_DIR, 'predictions')}")
