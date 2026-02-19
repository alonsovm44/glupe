import os
import bcrypt
import psycopg2
import jwt
import datetime
from flask import Flask, request, jsonify, send_file, abort
from flask_cors import CORS
from werkzeug.utils import secure_filename
from dotenv import load_dotenv

load_dotenv() # Load environment variables

app = Flask(__name__)
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'your_secret_key')
CORS(app)

# --- DATABASE CONFIGURATION ---
# Railway provides a DATABASE_URL environment variable automatically
DATABASE_URL = os.environ.get('DATABASE_URL')
STORAGE_DIR = os.environ.get('STORAGE_DIR', 'storage')

# Ensure storage directory exists
os.makedirs(STORAGE_DIR, exist_ok=True)

def get_db_connection():
    conn = psycopg2.connect(DATABASE_URL)
    return conn

# Initialize DB Table on startup
def init_db():
    conn = get_db_connection()
    cur = conn.cursor()
    # Create users table
    cur.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(100) NOT NULL
        );
    ''')
    conn.commit()
    cur.close()
    conn.close()

# Initialize on startup
try:
    init_db()
    print("[DB] Database initialized.")
except Exception as e:
    print(f"[DB] Error initializing: {e}")

# --- AUTH ENDPOINTS ---

# 1. REGISTER
@app.route('/register', methods=['POST'])
def register():
    data = request.json
    username = data.get('username')
    password = data.get('password')

    if not username or not password:
        return jsonify({"error": "Missing fields"}), 400

    # Hash password
    hashed = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())

    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("INSERT INTO users (username, password_hash) VALUES (%s, %s)", (username, hashed.decode('utf-8')))
        conn.commit()
        cur.close()
        conn.close()
        
        # Create user folder in storage
        user_folder = os.path.join(STORAGE_DIR, secure_filename(username))
        os.makedirs(user_folder, exist_ok=True)
        
        return jsonify({"status": "success", "message": "User created"}), 201
    except psycopg2.errors.UniqueViolation:
        return jsonify({"error": "User already exists"}), 409
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# 2. LOGIN (Returns success if password matches)
@app.route('/login', methods=['POST'])
def login():
    data = request.json
    username = data.get('username')
    password = data.get('password')

    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT password_hash FROM users WHERE username = %s", (username,))
    result = cur.fetchone()
    cur.close()
    conn.close()

    if result is None:
        return jsonify({"error": "User not found"}), 404

    stored_hash = result[0]
    if bcrypt.checkpw(password.encode('utf-8'), stored_hash.encode('utf-8')):
        token = jwt.encode({
            'user': username,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=24)
        }, app.config['SECRET_KEY'], algorithm="HS256")
        return jsonify({"status": "success", "token": token}), 200
    else:
        return jsonify({"error": "Invalid password"}), 401

# --- FILE ENDPOINTS ---

# 3. PUSH (Upload)
@app.route('/push', methods=['POST'])
def push_file():
    token = None
    if 'Authorization' in request.headers:
        auth_header = request.headers['Authorization']
        if auth_header.startswith("Bearer "):
            token = auth_header.split(" ")[1]

    if not token:
        return jsonify({"error": "Token is missing"}), 401

    try:
        data = jwt.decode(token, app.config['SECRET_KEY'], algorithms=["HS256"])
        current_user = data['user']
    except:
        return jsonify({"error": "Token is invalid"}), 401
    
    if 'file' not in request.files:
        return jsonify({"error": "No file"}), 400

    file = request.files['file']
    author = request.form.get('author', 'anonymous')
    
    if author != current_user:
        return jsonify({"error": "Unauthorized: Author mismatch"}), 403

    folder = request.form.get('folder', '') # Optional subfolder

    if file.filename == '':
        return jsonify({"error": "No filename"}), 400

    # Secure paths
    author_safe = secure_filename(author)
    filename_safe = secure_filename(file.filename)
    
    # Construct path: storage/author/folder/file.glp
    base_path = os.path.join(STORAGE_DIR, author_safe)
    if folder:
        base_path = os.path.join(base_path, secure_filename(folder))
    
    os.makedirs(base_path, exist_ok=True)
    
    file_path = os.path.join(base_path, filename_safe)
    file.save(file_path)
    
    return jsonify({"status": "success", "id": f"{author}/{folder}/{filename_safe}"})

# 4. PULL (Download)
@app.route('/pull/<path:file_id>', methods=['GET'])
def pull_file(file_id):
    # file_id example: alonso44/mytodo.glp OR alonso44/myfolder/mytodo.glp
    
    # Security check
    if ".." in file_id or file_id.startswith("/"):
        return abort(403)

    file_path = os.path.join(STORAGE_DIR, file_id)
    
    if not os.path.exists(file_path):
        return jsonify({"error": "File not found"}), 404

    return send_file(file_path)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)