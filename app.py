import os
import sys
import json
import subprocess
from flask import Flask, request, jsonify, render_template

app = Flask(__name__, static_folder="static", template_folder="templates")

# Determine binary name based on operating system
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BINARY_NAME = "dsa_core.exe" if sys.platform.startswith("win") else "dsa_core"
BINARY_PATH = os.path.join(BASE_DIR, BINARY_NAME)

def ensure_binary_compiled():
    """Ensure dsa_core binary exists; compile if missing."""
    if not os.path.exists(BINARY_PATH):
        print(f">> [INFO] Binary not found at {BINARY_PATH}. Compiling C DSA core...")
        sources = [
            "c_src/linked_list.c", "c_src/stack.c", "c_src/queue.c",
            "c_src/hash_table.c", "c_src/bst.c", "c_src/graph.c",
            "c_src/sorting_searching.c", "c_src/file_io.c", "c_src/main.c"
        ]
        cmd = ["gcc", "-Wall", "-Wextra", "-O2"] + sources + ["-o", BINARY_PATH]
        subprocess.run(cmd, cwd=BASE_DIR, check=True)
        print(">> [SUCCESS] C DSA core compiled successfully!")

def run_dsa_core(*args):
    """Invoke the compiled C DSA core in JSON API mode."""
    ensure_binary_compiled()
    cmd = [BINARY_PATH, "--api"] + [str(a) for a in args]
    try:
        result = subprocess.run(
            cmd,
            cwd=BASE_DIR,
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode != 0:
            return {
                "status": "error",
                "message": f"C engine returned code {result.returncode}",
                "stderr": result.stderr.strip()
            }
        stdout_clean = result.stdout.strip()
        # In case there are multiple lines, find the last JSON line
        lines = [line.strip() for line in stdout_clean.splitlines() if line.strip()]
        for line in reversed(lines):
            if line.startswith("{") and line.endswith("}"):
                return json.loads(line)
        return {"status": "raw", "output": stdout_clean}
    except subprocess.TimeoutExpired:
        return {"status": "error", "message": "C engine timed out"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# =============================================================================
# WEB DASHBOARD ROUTE
# =============================================================================

@app.route("/")
def index():
    return render_template("index.html")

# =============================================================================
# REST API ENDPOINTS (BRIDGED TO C DSA ENGINE)
# =============================================================================

@app.route("/api/students", methods=["GET"])
def get_students():
    """Retrieve all students stored in the C singly linked list."""
    res = run_dsa_core("list")
    return jsonify(res)

@app.route("/api/students", methods=["POST"])
def add_student():
    """Insert student into C linked list and hash table."""
    data = request.get_json(force=True)
    roll = data.get("roll")
    name = data.get("name", "").strip()
    dept = data.get("dept", "").strip()
    cgpa = data.get("cgpa", 0.0)
    email = data.get("email", "").strip()
    phone = data.get("phone", "").strip()

    if not roll or not name or not dept:
        return jsonify({"status": "error", "message": "Roll No, Name, and Department are required"}), 400

    res = run_dsa_core("add", roll, name, dept, cgpa, email, phone)
    status_code = 200 if res.get("status") == "success" else 400
    return jsonify(res), status_code

@app.route("/api/students/<int:roll>", methods=["PUT"])
def update_student(roll):
    """Update student record and push previous state to C Undo Stack."""
    data = request.get_json(force=True)
    name = data.get("name", "").strip()
    dept = data.get("dept", "").strip()
    cgpa = data.get("cgpa", 0.0)
    email = data.get("email", "").strip()
    phone = data.get("phone", "").strip()

    res = run_dsa_core("update", roll, name, dept, cgpa, email, phone)
    status_code = 200 if res.get("status") == "success" else 404
    return jsonify(res), status_code

@app.route("/api/students/<int:roll>", methods=["DELETE"])
def delete_student(roll):
    """Delete student record from C linked list and push to C Undo Stack."""
    res = run_dsa_core("delete", roll)
    status_code = 200 if res.get("status") == "success" else 404
    return jsonify(res), status_code

@app.route("/api/students/search/roll/<int:roll>", methods=["GET"])
def search_student_roll(roll):
    """O(1) Hash Table lookup + Binary Search validation in C."""
    res = run_dsa_core("search_roll", roll)
    return jsonify(res)

@app.route("/api/students/search/name", methods=["GET"])
def search_student_name():
    """Linear Search by substring name in C."""
    q = request.args.get("q", "").strip()
    if not q:
        return jsonify({"status": "success", "count": 0, "results": []})
    res = run_dsa_core("search_name", q)
    return jsonify(res)

@app.route("/api/students/sort/cgpa", methods=["GET"])
def sort_students_cgpa():
    """Merge Sort by CGPA executed in C."""
    order = request.args.get("order", "desc")
    res = run_dsa_core("sort_cgpa", order)
    return jsonify(res)

@app.route("/api/students/sort/name", methods=["GET"])
def sort_students_name():
    """Merge Sort alphabetically by Name executed in C."""
    order = request.args.get("order", "asc")
    res = run_dsa_core("sort_name", order)
    return jsonify(res)

@app.route("/api/undo", methods=["POST"])
def undo_action():
    """Pop previous state from C Undo Stack (LIFO) and restore record."""
    res = run_dsa_core("undo")
    status_code = 200 if res.get("status") == "success" else 400
    return jsonify(res), status_code

@app.route("/api/requests", methods=["GET"])
def list_requests():
    """List pending certificate requests from C FIFO Queue."""
    res = run_dsa_core("req_list")
    return jsonify(res)

@app.route("/api/requests", methods=["POST"])
def enqueue_request():
    """Enqueue new certificate ticket into C FIFO Queue."""
    data = request.get_json(force=True)
    roll = data.get("roll")
    cert_type = data.get("type", "Bonafide Certificate").strip()
    if not roll:
        return jsonify({"status": "error", "message": "Roll No is required"}), 400
    res = run_dsa_core("req_enqueue", roll, cert_type)
    return jsonify(res)

@app.route("/api/requests", methods=["DELETE"])
def dequeue_request():
    """Process/dequeue next ticket from front of C FIFO Queue."""
    res = run_dsa_core("req_dequeue")
    status_code = 200 if res.get("status") == "success" else 400
    return jsonify(res), status_code

@app.route("/api/departments", methods=["GET"])
def get_department_bst():
    """In-order traversal of Department Binary Search Tree in C."""
    res = run_dsa_core("departments_bst")
    return jsonify(res)

@app.route("/api/prerequisites", methods=["GET"])
def get_prerequisites_graph():
    """Curriculum DAG nodes, directed edges, and Kahn's Topological Sort in C."""
    res = run_dsa_core("prerequisites_graph")
    return jsonify(res)

# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    ensure_binary_compiled()
    port = int(os.environ.get("PORT", 5000))
    print(f">> [STARTING] Student Record Management System on http://127.0.0.1:{port}")
    app.run(host="0.0.0.0", port=port, debug=True)
