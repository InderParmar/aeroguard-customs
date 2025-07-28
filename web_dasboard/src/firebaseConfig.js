import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";
import { getAuth } from "firebase/auth";

const firebaseConfig = {
  apiKey: "AIzaSyCucDhKwRdWTnzNkTIdu2ci2rFRyzIADtA",
  authDomain: "aero-guard.firebaseapp.com",
  databaseURL: "https://aero-guard-default-rtdb.firebaseio.com",
  projectId: "aero-guard",
  storageBucket: "aero-guard.appspot.com",
  messagingSenderId: "739356087732",
  appId: "1:739356087732:web:6f69e4b94fb6e5f8858ee1"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);
const firebaseAuth = getAuth(app);

export { db, firebaseAuth };