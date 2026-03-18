from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess
import os
import json

app = Flask(__name__)
CORS(app)

@app.route('/simulate', methods=['POST'])
def simulate():
    data = request.json
    mode = data.get('mode', 'simulate')

    exe_path = os.path.join("backend", "engine")
    if os.name == 'nt': exe_path += ".exe"

    try:
        # 🔹 MEMORY / NORMAL MODE
        if mode == "simulate":
            algo = data.get('algo', 'first')
            blocks = ",".join(data['blocks'].split())
            procs = ",".join(data['procs'].split())

            result = subprocess.run(
                [exe_path, algo, blocks, procs],
                capture_output=True,
                text=True
            )

        # 🔹 COMPARISON MODE
        elif mode == "compare":
            blocks = ",".join(data['blocks'].split())
            procs = ",".join(data['procs'].split())

            result = subprocess.run(
                [exe_path, "compare", blocks, procs],
                capture_output=True,
                text=True
            )

        # 🔹 PAGING MODE
        elif mode == "paging":
            ref = ",".join(data['ref'].split())
            frames = str(data['frames'])
            algo = data.get('algorithm', 'fifo')

            result = subprocess.run(
                [exe_path, "paging", algo, ref, frames],
                capture_output=True,
                text=True
            )

        else:
            return jsonify({"error": "Invalid mode"}), 400

        # 🔥 SAFE JSON RETURN
        import json
        return jsonify(json.loads(result.stdout))

    except Exception as e:
        print("ERROR:", e)
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    print("Bridge Server running on http://localhost:5000")
    app.run(port=5000)