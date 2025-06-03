from ultralytics import YOLO

# Load pretrained model
# model = YOLO("yolov8n.pt")
model = YOLO("new_pretrain.pt")

# Run inference
results = model("weaponsgta5.gif")

# Access the first result in the list
res = results[0]

# Show and save the image with bounding boxes
res.show()
res.save()
