/**
 * Cloud Function: processImage
 * Trigger: GCS finalize (Firebase Storage upload)
 * Flow:
 *   1) Download the uploaded image to /tmp
 *   2) Send it to the Cloud Run inference API
 *   3) Write the top prediction to Realtime DB:
 *        - /predictions/latest
 *        - /predictions/history/<timestamp>
 *
 * Note:
 * - Set PREDICT_API_URL as an environment variable
 */
// Cloud Run API endpoint
const PREDICT_API_URL = process.env.PREDICT_API_URL;
const {initializeApp} = require("firebase-admin/app");
const {getStorage} = require("firebase-admin/storage");
const {getDatabase} = require("firebase-admin/database");
const {onObjectFinalized} = require("firebase-functions/v2/storage");
const {tmpdir} = require("os");
const path = require("path");
const fs = require("fs");
const FormData = require("form-data");
const axios = require("axios");
// Initialize Firebase Admin SDK
// (uses service account from Functions environment)

initializeApp();
// Cloud Function: runs when a new file is finalized in the bucket
exports.processImage = onObjectFinalized(
    {region: "us-central1", memory: "256MiB", cpu: 1},
    async (cloudEvent) => {
      const object = cloudEvent.data;
      const filePath = object.name;
      const bucketName = object.bucket;
      // Guard clause: we need both the file path and bucket name
      if (!filePath || !bucketName) {
        console.error("Missing name or bucket in event.data:", object);
        return;
      }
      console.log("New file:", filePath);
      // Download the uploaded image to local temp path
      const bucket = getStorage().bucket(bucketName);
      const tempFilePath = path.join(tmpdir(), path.basename(filePath));

      try {
      // 1) Download from Cloud Storage to /tmp
        await bucket.file(filePath).download({destination: tempFilePath});
        console.log("Downloaded to", tempFilePath);

        // 2) send to Cloud Run Inference API
        const form = new FormData();
        form.append("file", fs.createReadStream(tempFilePath));
        const {data} = await axios.post(PREDICT_API_URL, form, {
          headers: form.getHeaders(),
        });
        // fallback to results if predictions is undefined
        const allPreds = Array.isArray(data.predictions) ?
        data.predictions :
        Array.isArray(data.results) ?
        data.results :
        [];
        console.log("Raw predictions:", allPreds);

        // 3) Choose the top prediction by confidence (or a null record if none)
        let toSave;
        if (allPreds.length > 0) {
          const top = allPreds.reduce((best, cur) =>
          cur.confidence > best.confidence ? cur : best,
          );
          toSave = {
            image: filePath,
            label: top.label,
            confidence: top.confidence,
            bbox: top.bbox,
            timestamp: Math.floor(Date.now() / 1000),
          };
        } else {
          toSave = {
            image: filePath,
            label: null,
            confidence: null,
            bbox: null,
            timestamp: Math.floor(Date.now() / 1000),
          };
        }
        console.log("Top prediction:", toSave);

        // 4 Persist results in Realtime Database: write latest and
        // append to history
        const db = getDatabase();
        // Overwrite the latest record so the dashboard can read quickly
        await db.ref("/predictions/latest").set(toSave);
        console.log("Wrote top prediction to /predictions/latest");

        // Append to a history collection keyed by timestamp for later retrieval
        await db.ref(`/predictions/history/${toSave.timestamp}`).set(toSave);
        console.log("Appended to /predictions/history");
      } catch (err) {
        // Keep logs simple for CI and debugging
        console.error("Error in processImage:", err);
      }
    },
);
