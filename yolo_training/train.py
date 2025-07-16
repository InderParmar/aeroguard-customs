import os
import torch
from ultralytics import YOLO

def main():
    # --- Config ---
    model_arch = "runs/knife_gun_yolo3/weights/last.pt"  # Use yolov8m for best balance
    data_yaml = "dataset.yaml"
    epochs = 50
    batch = 16
    imgsz = 640
    project = "runs"
    name = "knife_gun_yolo3"

    # --- Device Detection ---
    device = 0 if torch.cuda.is_available() else "cpu"
    print("CUDA Available:", torch.cuda.is_available())
    if torch.cuda.is_available():
        print("GPU Detected:", torch.cuda.get_device_name(0))

    # --- Load model ---
    model = YOLO(model_arch)

    # --- Train ---
    model.train(
        data=data_yaml,
        imgsz=imgsz,
        epochs=epochs,
        batch=batch,
        name="knife_gun_yolo4",
        project=project,
        optimizer="SGD",
        augment=True,
        hsv_h=0.015,
        hsv_s=0.7,
        hsv_v=0.4,
        degrees=5.0,
        translate=0.1,
        scale=0.5,
        fliplr=0.5,
        mosaic=1.0,
        mixup=0.2,
        patience=20,
        val=True,
        save=True,
        save_period=5,
        verbose=True,
        save_json=True,
        device=device,
        amp=True,
        resume=False
    )

    # --- Evaluate ---
    metrics = model.val()
    print("Evaluation Metrics:")
    print(metrics)

    # --- Export to ONNX ---
    print("Exporting model to ONNX and TorchScript...")
    model.export(format="onnx")         # Exports to ONNX
    model.export(format="torchscript")  # Also export to TorchScript


if __name__ == "__main__":
    main()
