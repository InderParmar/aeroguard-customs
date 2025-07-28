// import React, { useEffect, useState } from "react";
// import { onAuthStateChanged, signOut } from "firebase/auth";
// import { ref, onValue } from "firebase/database";
// import { firebaseAuth, db } from "./firebaseConfig";
// import Login from "./Login";
// import "./App.css";

// const audio = new Audio(process.env.PUBLIC_URL + "/assets/alert.mp3");

// function App() {
//   const [user, setUser] = useState(null);
//   const [detectionHistory, setDetectionHistory] = useState([]);
//   const [latestDetection, setLatestDetection] = useState(null);

//   useEffect(() => {
//     const unsubscribe = onAuthStateChanged(firebaseAuth, setUser);
//     return () => unsubscribe();
//   }, []);

//   useEffect(() => {
//     if (!user) return;

//     const latestRef = ref(db, "predictions/latest");
//     const historyRef = ref(db, "predictions/history");

//     onValue(latestRef, (snapshot) => {
//       const data = snapshot.val();
//       if (data) {
//         setLatestDetection(data);
//         if (data.label?.toLowerCase().includes("gun")) {
//           audio.play();
//         }
//       }
//     });

//     onValue(historyRef, (snapshot) => {
//       const historyData = snapshot.val();
//       if (historyData) {
//         const historyArray = Object.values(historyData).sort(
//           (a, b) => b.timestamp - a.timestamp
//         );
//         setDetectionHistory(historyArray);
//       }
//     });
//   }, [user]);

//   const handleLogout = () => signOut(firebaseAuth);

//   const renderCard = (detection, isLatest = false) => {
//     const formattedTime = detection.timestamp
//       ? new Date(detection.timestamp * 1000).toLocaleString()
//       : "Invalid";

//     const isHighRisk = detection.label?.toLowerCase().includes("gun");

//     // Convert relative path to full Firebase Storage URL
//     const fullImageUrl = detection.image?.startsWith("http")
//       ? detection.image
//       : `https://firebasestorage.googleapis.com/v0/b/aero-guard.appspot.com/o/${encodeURIComponent(detection.image)}?alt=media`;

//     return (
//       <div
//         key={detection.timestamp}
//         className={`card ${isHighRisk ? "danger" : ""} ${isLatest ? "latest" : ""}`}
//       >
//         <p><strong>Label:</strong> {detection.label || "Unavailable"}</p>
//         <p><strong>Confidence:</strong> {detection.confidence !== undefined ? (detection.confidence * 100).toFixed(2) + "%" : "Unavailable"}</p>
//         <p><strong>Timestamp:</strong> {formattedTime}</p>
//         {detection.image && (
//           <img src={fullImageUrl} alt={detection.label} className="preview-image" />
//         )}
//       </div>
//     );
// };


//   if (!user) return <Login onLogin={setUser} />;

//   return (
//     <div className="dashboard">
//       <div style={{ textAlign: "right", marginBottom: "20px" }}>
//         <button
//           onClick={handleLogout}
//           style={{
//             backgroundColor: "#c0392b",
//             color: "#fff",
//             border: "none",
//             padding: "10px 20px",
//             borderRadius: "5px",
//             fontSize: "14px",
//             fontWeight: "bold",
//             cursor: "pointer",
//             transition: "background-color 0.3s ease"
//           }}
//           onMouseOver={(e) => (e.target.style.backgroundColor = "#a93226")}
//           onMouseOut={(e) => (e.target.style.backgroundColor = "#c0392b")}
//         >
//           Sign Out
//         </button>
//       </div>

//       <h1>AeroGuard Customs – Hazard Detection Dashboard</h1>

//       <section className="latest-section">
//         <h2>Latest Detection</h2>
//         {latestDetection ? renderCard(latestDetection, true) : <p>No latest detection.</p>}
//       </section>

//       <section className="history-section">
//         <h2>Detection History</h2>
//         {detectionHistory.length > 0 ? (
//           detectionHistory.map((d) => renderCard(d))
//         ) : (
//           <p>No history available.</p>
//         )}
//       </section>
//     </div>
//   );
// }

// export default App;

import React, { useEffect, useState } from "react";
import { onAuthStateChanged, signOut } from "firebase/auth";
import { ref, onValue } from "firebase/database";
import { firebaseAuth, db } from "./firebaseConfig";
import Login from "./Login";
import "./App.css";

const audio = new Audio(process.env.PUBLIC_URL + "/assets/alert.mp3");

function App() {
  const [user, setUser] = useState(null);
  const [detectionHistory, setDetectionHistory] = useState([]);
  const [latestDetection, setLatestDetection] = useState(null);

  useEffect(() => {
    const unsubscribe = onAuthStateChanged(firebaseAuth, setUser);
    return () => unsubscribe();
  }, []);

  useEffect(() => {
    if (!user) return;

    const latestRef = ref(db, "predictions/latest");
    const historyRef = ref(db, "predictions/history");

    onValue(latestRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        setLatestDetection(data);
        if (data.label?.toLowerCase().includes("gun")) {
          audio.play().catch((err) => console.error("Audio error:", err));
        }
      }
    });

    onValue(historyRef, (snapshot) => {
      const historyData = snapshot.val();
      if (historyData) {
        const historyArray = Object.values(historyData).sort(
          (a, b) => b.timestamp - a.timestamp
        );
        setDetectionHistory(historyArray);
      }
    });
  }, [user]);

  const handleLogout = () => signOut(firebaseAuth);

  const renderCard = (detection, isLatest = false) => {
    const formattedTime = detection.timestamp
      ? new Date(detection.timestamp * 1000).toLocaleString()
      : "Invalid";

    const isHighRisk = detection.label?.toLowerCase().includes("gun");

    // Firebase Storage URL Construction
    const imagePath = detection.image || "";
    const fullImageUrl = imagePath.startsWith("http")
      ? imagePath
      : `https://firebasestorage.googleapis.com/v0/b/aero-guard.appspot.com/o/${encodeURIComponent(
          imagePath
        )}?alt=media`;

    return (
      <div
        key={detection.timestamp}
        className={`card ${isHighRisk ? "danger" : ""} ${isLatest ? "latest" : ""}`}
      >
        <p><strong>Label:</strong> {detection.label || "Unavailable"}</p>
        <p><strong>Confidence:</strong> {detection.confidence !== undefined
          ? (detection.confidence * 100).toFixed(2) + "%"
          : "Unavailable"}
        </p>
        <p><strong>Timestamp:</strong> {formattedTime}</p>
        {imagePath && (
          <img
            src={fullImageUrl}
            alt={detection.label || "Detected Item"}
            className="preview-image"
            onError={(e) => {
              console.warn("Image failed to load:", fullImageUrl);
              e.target.style.display = "none"; // hide broken image
            }}
          />
        )}
      </div>
    );
  };

  if (!user) return <Login onLogin={setUser} />;

  return (
    <div className="dashboard">
      <div style={{ textAlign: "right", marginBottom: "20px" }}>
        <button
          onClick={handleLogout}
          style={{
            backgroundColor: "#c0392b",
            color: "#fff",
            border: "none",
            padding: "10px 20px",
            borderRadius: "5px",
            fontSize: "14px",
            fontWeight: "bold",
            cursor: "pointer",
            transition: "background-color 0.3s ease"
          }}
          onMouseOver={(e) => (e.target.style.backgroundColor = "#a93226")}
          onMouseOut={(e) => (e.target.style.backgroundColor = "#c0392b")}
        >
          Sign Out
        </button>
      </div>

      <h1>AeroGuard Customs – Hazard Detection Dashboard</h1>

      <section className="latest-section">
        <h2>Latest Detection</h2>
        {latestDetection ? renderCard(latestDetection, true) : <p>No latest detection.</p>}
      </section>

      <section className="history-section">
        <h2>Detection History</h2>
        {detectionHistory.length > 0
          ? detectionHistory.map((d) => renderCard(d))
          : <p>No history available.</p>}
      </section>
    </div>
  );
}

export default App;
