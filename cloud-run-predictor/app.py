from fastapi import FastAPI, File, UploadFile, HTTPException
import torch
from torchvision import transforms
from PIL import Image
import io
import os

app = FastAPI()

# Simple health check endpoint for Cloud Run and uptime checks
@app.get("/")
def health_check():
    return {"status": "ok"}

# Model class names (must match training config)
CLASS_NAMES = ["Gun Detected", "Knife Detected"]

# Load the TorchScript model once at startup (CPU)
try:
    print("Loading model...")
    model = torch.jit.load("best_nms.torchscript", map_location=torch.device("cpu"))
    model.eval()
    print("Model loaded successfully.")
except Exception as e:
    print(f"Model load failed: {e}")
    raise RuntimeError(f"Failed to load model: {e}")

@app.post("/predict")
async def predict(file: UploadFile = File(...)):
    """
    Accepts an image file, runs inference, and returns detections as JSON.
    """
    try:
        # Read and preprocess the input image
        image_bytes = await file.read()
        image = Image.open(io.BytesIO(image_bytes)).convert("RGB")

        transform = transforms.Compose([
            transforms.Resize((640, 640)),
            transforms.ToTensor()
        ])
        input_tensor = transform(image).unsqueeze(0)  # shape: [1, 3, 640, 640]

        # Inference (expects model output with shape [N, 6])
        with torch.no_grad():
            output = model(input_tensor)[0]

        # Post-process predictions
        predictions = []
        for pred in output:
            vals = pred.tolist()
            if len(vals) < 6:
                continue
            x1, y1, x2, y2, conf, cls = vals[:6]
            if conf < 0.5:
                continue
            label = CLASS_NAMES[int(cls)] if int(cls) < len(CLASS_NAMES) else f"class_{int(cls)}"
            predictions.append({
                "label": label,
                "confidence": round(conf, 4),
                "bbox": {
                    "x1": round(x1, 2),
                    "y1": round(y1, 2),
                    "x2": round(x2, 2),
                    "y2": round(y2, 2),
                }
            })

        return {"predictions": predictions}

    except Exception as e:
        print(f"Prediction error: {e}")
        raise HTTPException(status_code=500, detail=f"Prediction failed: {e}")

# Entry point for local development (Cloud Run ignores this)
if __name__ == "__main__":
    import uvicorn
    uvicorn.run("app:app", host="0.0.0.0", port=int(os.getenv("PORT", 8080)))
