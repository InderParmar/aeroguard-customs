import React, { useState } from "react";
import { signInWithEmailAndPassword } from "firebase/auth";
import { firebaseAuth } from "./firebaseConfig";
import "./App.css";

const Login = ({ onLogin }) => {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [errorMsg, setErrorMsg] = useState("");

  const handleLogin = async (e) => {
    e.preventDefault();
    setErrorMsg("");

    //testuer@gmail.com -> test123
    try {
      const userCredential = await signInWithEmailAndPassword(firebaseAuth, email, password);
      onLogin(userCredential.user);
    } catch (error) {
      setErrorMsg("Invalid email or password.");
      console.error("Login failed:", error);
    }
  };

  return (
    <div className="login-form">
      <h2>AEROGUARD CUSTOMS</h2>
      <h3>LOGIN</h3>

      <form onSubmit={handleLogin}>
        <input
          type="email"
          placeholder="Email address"
          value={email}
          onChange={(e) => setEmail(e.target.value)}
          required
        />
        <input
          type="password"
          placeholder="Password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          required
        />
        <button type="submit">Login</button>
        {errorMsg && <p className="error">{errorMsg}</p>}
      </form>
    </div>
  );
};

export default Login;
