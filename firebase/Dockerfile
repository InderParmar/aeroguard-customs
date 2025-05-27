FROM python:3.9-slim

# Install system dependencies required by OpenCV and TensorFlow
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy all files into the container
COPY . .

# Upgrade pip and install dependencies
RUN pip install --upgrade pip
RUN pip install -r requirements.txt

# Expose port 8080 for Cloud Run
EXPOSE 8080

# Start the Flask app using Gunicorn on port 8080
CMD ["gunicorn", "-b", ":8080", "app:app"]
