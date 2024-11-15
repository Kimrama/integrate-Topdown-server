import base64
from datetime import datetime
from flask import Flask, jsonify, request, send_file
from pymongo import MongoClient
from io import BytesIO
import gridfs
from dotenv import load_dotenv
import os

load_dotenv()

app = Flask(__name__)

mongo_uri = "mongodb://mongodb_container:27017"

matlab_key = os.getenv("API_KEY_MATHLAB")
raspi_key = os.getenv("API_KEY_RASPI")

client = MongoClient(mongo_uri)
db = client["integration"]

collection = db["test"]
file_collection = db["test_file"]
machine_collection = db["machine"]

fs = gridfs.GridFS(db)
@app.route('/projects', methods=['GET'])
def get_projects():
    return jsonify({'projects': "Topdown"})

@app.route('/databases', methods=['GET'])
def show_databases():
    try:
        databases = client.list_database_names()
        return jsonify({"databases": databases}), 200
    except Exception as e:
        return jsonify({"message": "Failed to fetch databases", "error": str(e)}), 500

@app.route('/add_item', methods=['POST'])
def add_item():

    item_data = request.json
    collection.insert_one(item_data)
    return jsonify({"message": "Item added successfully"}), 201




    
@app.route('/files', methods=['GET'])
def list_files():
    api_key = request.headers.get('APIKEY');
    try:
        files = fs.find()
        file_names = [file.filename for file in files]
        print("".format(file_names))

        if not file_names:
            return jsonify({"message": "No files found"}), 404

        return jsonify({"files": file_names}), 200

    except Exception as e:
        return jsonify({"message": f"An error occurred: {str(e)}"}), 500

@app.route('/upload', methods=['POST'])
def upload_file():
    try:
        data = request.get_json()
        if not data or 'file_name' not in data or 'data' not in data:
            return jsonify({"message": "Invalid request format"}), 400

        file_name = data['file_name']
        base64_data = data['data']

        file_content = base64.b64decode(base64_data)

        if fs.exists({"filename": file_name}):
            return jsonify({"message": "File with the same name already exists"}), 400

        fs.put(file_content, filename=file_name)

        return jsonify({"message": "File uploaded successfully"}), 201

    except Exception as e:
        return jsonify({"message": f"An error occurred: {str(e)}"}), 500
@app.route('/download/<file_name>', methods=['GET'])
def download_file(file_name):
    try:
        file = fs.find_one({"filename": file_name})
        
        if not file:
            return jsonify({"message": "File not found"}), 404
        
        response = send_file(
            BytesIO(file.read()),
            mimetype='application/octet-stream',
            as_attachment=True,
            download_name=file.filename
        )
        
        return response
    
    except Exception as e:
        return jsonify({"message": str(e)}), 500
    
    except gridfs.errors.NoFile:
        return jsonify({"message": "File not found"}), 404
    
@app.route('/get_all_collections', methods=['GET'])
def get_all_collections():
    try:
        collections = db.list_collection_names()

        return jsonify(collections), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500
    
@app.route('/get_all_data', methods=['GET'])
def get_all_data():
    try:
        results = machine_collection.find()
        

        data = [doc for doc in results]
        for item in data:
            item['_id'] = str(item['_id']) 
        
        return jsonify(data), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500
    
@app.route('/get_data_by_timestamp', methods=['GET'])
def get_data_by_timestamp():
    try:
        timestamp = request.args.get('timestamp') 

        if not timestamp:
            return jsonify({'error': 'Please provide a timestamp in dd/mm/yyyy hh:mm:ss format'}), 400

        result = machine_collection.find_one({'timestamp': timestamp})

        if result:
            result['_id'] = str(result['_id'])  
            return jsonify(result), 200
        else:
            return jsonify({'message': 'Document not found'}), 404

    except Exception as e:
        return jsonify({'error': str(e)}), 500 

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)


