# 🕸️ Project GhostNet: Cognitive LLM-Powered Deception

GhostNet is an advanced, high-fidelity deception platform that uses **Groq-accelerated LLMs** (Moonshot Kimi-k2) to hallucinate realistic, stateful attack surfaces in real-time. It traps attackers in an infinite, semi-autonomous maze while streaming full telemetry to a centralized **Wazuh SIEM**.

---

## 🚀 Quick Start

1.  **Clone the repository**
    ```bash
    git clone https://github.com/khalilammarr/LLM_Honeypot.git
    cd GhostNet
    ```

2.  **Configure Environment**
    Copy the example environment file and add your Groq API key:
    ```bash
    cp .env.example .env
    # Edit .env and add API_KEY
    ```

3.  **Deploy with Docker**
    ```bash
    docker compose up -d --build
    ```

---

## 🛠️ Core Capabilities

*   **Cognitive SSH (Cowrie + Groq):** Fully interactive Bash shell that hallucinates file systems, scripts, and logs in real-time.
*   **Dynamic HTTP (FastAPI):** Adaptive web portal that generates realistic configuration files, database dumps, and API responses.
*   **Zero-Leaking Deception:** Custom Python filters strip model-specific reasoning tags, ensuring the attacker only sees raw terminal output.
*   **SIEM Integration:** Native integration with Wazuh for real-time alerting and threat analysis.

---

## 📁 Repository Structure

*   `configs/`: Global honeypot behavior and protocol definitions.
*   `honeypot/`: Core service source code (HTTP/Agent).
*   `infra/`: SIEM and monitoring configuration (Wazuh/Grafana).
*   `docs/`: Detailed architectural reports and PDF technical specifications.

---

## ⚠️ Safety Notice

> [!CAUTION]
> This is an active deception lab. Do not host these services un-sandboxed on the public internet. Observe traffic in Host-Only or strictly firewalled environments.
