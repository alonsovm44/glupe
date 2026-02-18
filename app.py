from flask import Flask, request, jsonify, send_file
import os
from werkzeug.utils import secure_filename

app = Flask(__name__)
# The folder where "Program Images" are stored
STORAGE_DIR = 'storage'
os.makedirs(STORAGE_DIR, exist_ok=True)

# 1. PUSH: Upload a file
@app.route('/push', methods=['POST'])
def push_file():
    # We expect a file and an optional author name
    if 'file' not in request.files:
        return jsonify({"error": "No file provided"}), 400
    
    file = request.files['file']
    author = request.form.get('author', 'anonymous')
    
    if file.filename == '':
        return jsonify({"error": "No filename"}), 400

    # Secure the filename (prevent hacks)
    filename = secure_filename(file.filename)
    
    # Create a folder for the author (simple organization)
    author_dir = os.path.join(STORAGE_DIR, author)
    os.makedirs(author_dir, exist_ok=True)
    
    # Save the file
    save_path = os.path.join(author_dir, filename)
    file.save(save_path)
    
    return jsonify({
        "status": "success", 
        "message": f"File saved as {author}/{filename}",
        "id": f"{author}/{filename}"
    }), 200

# 2. PULL: Download a file
@app.route('/pull/<path:file_id>', methods=['GET'])
def pull_file(file_id):
    # file_id looks like: "alonso44/utils.glp"
    try:
        # Security check to prevent directory traversal
        if ".." in file_id or file_id.startswith("/"):
            return jsonify({"error": "Invalid ID"}), 403

        file_path = os.path.join(STORAGE_DIR, file_id)
        
        if not os.path.exists(file_path):
            return jsonify({"error": "File not found"}), 404

        return send_file(file_path)
    
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# 3. SEARCH: List files (Basic)
@app.route('/search', methods=['GET'])
def search():
    files = []
    for author in os.listdir(STORAGE_DIR):
        author_path = os.path.join(STORAGE_DIR, author)
        if os.path.isdir(author_path):
            for fname in os.listdir(author_path):
                files.append(f"{author}/{fname}")
    return jsonify({"files": files}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)