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
         CREATE TABLE IF NOT EXISTS tags (
        id SERIAL PRIMARY KEY,
        file_id TEXT NOT NULL,
        tag VARCHAR(50) NOT NULL
    );
    ''')
    conn.commit()
    cur.close()
    conn.close()

# Initialize on startup
try:
    init_db()
    # Migration for existing users table
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("ALTER TABLE users ADD COLUMN IF NOT EXISTS email VARCHAR(100)")
        conn.commit()
        conn.close()
    except:
        pass
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
    
def get_user_from_token(token):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT username FROM tokens WHERE token = %s", (token,))
    result = cur.fetchone()
    cur.close()
    conn.close()
    if result:
        return result[0] # Return the username
    return None
# 2. LOGIN

# --- SIGNUP FLOW ---

@app.route('/auth/check_username', methods=['GET'])
def check_username():
    username = request.args.get('q', '').strip()
    if not username: return jsonify({"error": "Empty username"}), 400
    
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT 1 FROM users WHERE username = %s", (username,))
    exists = cur.fetchone()
    cur.close()
    conn.close()
    
    return jsonify({"available": not exists})

@app.route('/auth/signup', methods=['POST'])
def signup():
    data = request.json
    username = data.get('username')
    password = data.get('password')
    
    if not username or not password:
        return jsonify({"error": "Missing fields"}), 400

    hashed = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')
    
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("INSERT INTO users (username, password_hash) VALUES (%s, %s)", (username, hashed))
        conn.commit()
        
        user_folder = os.path.join(STORAGE_DIR, secure_filename(username))
        os.makedirs(user_folder, exist_ok=True)
        
        cur.close()
        conn.close()
        
        return jsonify({"status": "success", "message": "User created successfully"})
    except psycopg2.errors.UniqueViolation:
        return jsonify({"error": "Username already taken"}), 409
    except Exception as e:
        # This will catch DB errors or other unexpected issues
        print(f"[FATAL] An unhandled exception occurred in /auth/signup: {e}")
        return jsonify({"error": f"An unexpected server error occurred: {str(e)}"}), 500

# REPLACE YOUR login FUNCTION WITH THIS:

@app.route('/login', methods=['POST'])
def login():
    data = request.json
    username = data.get('username')
    password = data.get('password')

    if not username or not password:
        return jsonify({"error": "Missing fields"}), 400

    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT password_hash FROM users WHERE username = %s", (username,))
    result = cur.fetchone()
    cur.close()
    conn.close()

    if result is None:
        return jsonify({"error": "User not found"}), 404

    stored_hash = result[0]
    
    try:
        # Check password
        # Ensure input is bytes for bcrypt
        if bcrypt.checkpw(password.encode('utf-8'), stored_hash.encode('utf-8')):
            
            # --- JWT TOKEN GENERATION ---
            payload = {
                'user': username,
                'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=24)
            }
            
            token = jwt.encode(payload, app.config['SECRET_KEY'], algorithm="HS256")

            # PyJWT v2+ returns a string, but older versions returned bytes.
            if isinstance(token, bytes):
                token = token.decode('utf-8')
            
            return jsonify({"status": "success", "token": token}), 200
        else:
            return jsonify({"error": "Invalid password"}), 401
            
    except ValueError as ve:
        # This catches "Invalid salt" if the DB hash is corrupted/not bcrypt
        print(f"Bcrypt Error: {ve}")
        return jsonify({"error": "Database integrity error (Bad Hash)"}), 500
    except Exception as e:
        print(f"Error checking password: {e}")
        return jsonify({"error": f"Server error: {str(e)}"}), 500
    
# --- FILE ENDPOINTS --- 

# 3. PUSH (Upload)
@app.route('/push', methods=['POST'])
def push_file():
    # 1. Extract Token
    token = None
    if 'Authorization' in request.headers:
        auth_header = request.headers['Authorization']
        if auth_header.startswith("Bearer "):
            token = auth_header.split(" ")[1]

    if not token:
        return jsonify({"error": "Token is missing"}), 401

    # Get tags from form data (comma separated string)
    tags_str = request.form.get('tags', '') 
    tags_list = [t.strip().lower() for t in tags_str.split(',') if t.strip()]

    # 2. Verify Token
    try:
        # Decode the token using the secret key
        data = jwt.decode(token, app.config['SECRET_KEY'], algorithms=["HS256"])
        current_user = data['user']
    except jwt.ExpiredSignatureError:
        return jsonify({"error": "Token has expired"}), 401
    except jwt.InvalidTokenError:
        return jsonify({"error": "Token is invalid"}), 401
    except Exception as e:
        return jsonify({"error": f"Authentication failed: {str(e)}"}), 401
    
    # 3. Check File Presence
    if 'file' not in request.files:
        return jsonify({"error": "No file"}), 400

    file = request.files['file']
    
    # 4. Handle Author/Username
    # We trust the token, not the form data.
    # The CLI might send 'author' in form data, but we ignore it to prevent spoofing.
    author = current_user
    
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

    # Definir el ID del archivo que se usará en la tabla de tags
    # Usamos el mismo formato que devuelves en el JSON de éxito
    file_id = f"{author_safe}/{secure_filename(folder) if folder else ''}{filename_safe}".replace("//", "/")

    # Save tags to DB
    if tags_list:
        conn = get_db_connection()
        cur = conn.cursor()
        # Delete old tags for this file (in case of re-upload)
        cur.execute("DELETE FROM tags WHERE file_id = %s", (file_id,))
        # Insert new tags
        for tag in tags_list:
            cur.execute("INSERT INTO tags (file_id, tag) VALUES (%s, %s)", (file_id, tag))
        conn.commit()
        cur.close()
        conn.close()

    
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

# --- SEARCH ENDPOINTS ---

# 1. Search Users
@app.route('/search/users', methods=['GET'])
def search_users():
    query = request.args.get('q', '').lower()
    conn = get_db_connection()
    cur = conn.cursor()
    # Simple search: find usernames containing the query
    cur.execute("SELECT username FROM users WHERE LOWER(username) LIKE %s", (f'%{query}%',))
    users = [row[0] for row in cur.fetchall()]
    cur.close()
    conn.close()
    return jsonify({"users": users}), 200

# 2. Search Files
@app.route('/search/files', methods=['GET'])
def search_files():
    query = request.args.get('q', '').lower()
    tag_filter = request.args.get('tag', '').lower() #tags
    author = request.args.get('author', None) # Optional filter
    
    conn = get_db_connection()
    cur = conn.cursor()

    if tag_filter:
        # SEARCH BY TAG
        cur.execute("SELECT file_id FROM tags WHERE tag = %s", (tag_filter,))
        files = [row[0] for row in cur.fetchall()]
        cur.close()
        conn.close()
        # Return simplified list
        return jsonify({"files": [{"id": f, "filename": f.split('/')[-1]} for f in files]}), 200
    

    results = []
    
    # Walk through the storage directory
    for root, dirs, files in os.walk(STORAGE_DIR):
        for file in files:
            if file.endswith('.glp'):
                full_path = os.path.join(root, file)
                # Calculate relative path for ID (e.g., alonsovm/project/file.glp)
                rel_path = os.path.relpath(full_path, STORAGE_DIR)
                
                # Filter logic
                if query in rel_path.lower():
                    if author and not rel_path.startswith(author + os.sep):
                        continue
                    
                    # Extract metadata (simple version: just grab filename)
                    results.append({
                        "id": rel_path.replace("\\", "/"), # Normalize path
                        "filename": file
                    })

    return jsonify({"files": results}), 200

# 3. Get Metadata (Read file header)
@app.route('/meta/<path:file_id>', methods=['GET'])
def get_meta(file_id):
    file_path = os.path.join(STORAGE_DIR, file_id)
    if not os.path.exists(file_path):
        return jsonify({"error": "Not found"}), 404

    # Just read the first 20 lines to get META_START ... META_END
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = []
            for i, line in enumerate(f):
                if i < 30: lines.append(line)
                else: break
        return jsonify({"content_preview": "".join(lines)}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)