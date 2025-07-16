import os
import random
import shutil
from pathlib import Path

# Configuration
image_dir = Path(r"C:\Users\apatel477\Desktop\project\weapon_dataset\images")
label_dir = Path(r"C:\Users\apatel477\Desktop\project\weapon_dataset\labels")
split_ratio = 0.8  # 80% train, 20% val

# Output folders
train_img_dir = image_dir / "train"
val_img_dir = image_dir / "val"
train_lbl_dir = label_dir / "train"
val_lbl_dir = label_dir / "val"

# Create output directories
for path in [train_img_dir, val_img_dir, train_lbl_dir, val_lbl_dir]:
    path.mkdir(parents=True, exist_ok=True)

# Gather all image files with supported extensions
image_files = []
for ext in ("*.jpg", "*.jpeg", "*.png", "*.JPG", "*.JPEG", "*.PNG"):
    image_files.extend(image_dir.glob(ext))
random.shuffle(image_files)

# Split
split_index = int(len(image_files) * split_ratio)
train_files = image_files[:split_index]
val_files = image_files[split_index:]

# Copy function
def copy_files(file_list, dest_img_dir, dest_lbl_dir):
    for img_path in file_list:
        label_path = label_dir / img_path.with_suffix(".txt").name
        shutil.copy2(img_path, dest_img_dir / img_path.name)
        if label_path.exists():
            shutil.copy2(label_path, dest_lbl_dir / label_path.name)

# Perform copy
copy_files(train_files, train_img_dir, train_lbl_dir)
copy_files(val_files, val_img_dir, val_lbl_dir)

# Output summary
print("Dataset split completed.")
print(f"Training images: {len(train_files)}")
print(f"Validation images: {len(val_files)}")
