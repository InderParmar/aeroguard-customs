FROM python:3.9-slim

WORKDIR /app
COPY . .

COPY requirements.txt .
RUN pip install --upgrade pip
RUN pip install --no-cache-dir -r requirements.txt

COPY best_nms.torchscript best_nms.torchscript

EXPOSE 8080
CMD ["uvicorn", "app:app", "--host", "0.0.0.0", "--port", "8080"]
