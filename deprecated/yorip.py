import json
import sys
import os

# Importamos la NUEVA librería oficial
try:
    from google import genai
    from google.genai import types
except ImportError:
    print("❌ Error: No tienes la librería 'google-genai' instalada.")
    print("   Ejecuta: pip install google-genai")
    sys.exit(1)

# --- CONFIGURACIÓN ---
# PEGA TU API KEY AQUÍ (Consíguela en https://aistudio.google.com/)
API_KEY = "AIzaSyCBFQPavgZkZ0c0RDJmgJjv9LLL-1CpZ90" 

# Modelo a usar (Flash es rápido y gratis en el tier base)
MODEL_ID = "gemini-2.5-flash"
def get_system_prompt():
    return """
    You are the Master Compiler for the 'Yori' language. Your ONLY job is to convert Yori code into strict structural JSON.
    
    PARSING INSTRUCTIONS:
    1. Interpret code blocks as software modules.
    2. Process special comments (Directives) as follows:
       - '// >>>' -> Field 'design_intent': Infer visual styles, colors, and tone (Creativity).
       - '// !!!' -> Field 'critical_rules': Extract strict logic rules and validations (Security/Rigor).
       - '// ???' -> Field 'variants': Generate a list of 3 suggested technical options (Exploration).
    
    OUTPUT SCHEMA (JSON Only):
    {
      "meta": { "program_name": "string", "tech_stack": ["string"] },
      "modules": [
        {
          "name": "string",
          "type": "UI|LOGIC|DATA",
          "specs": { "key": "value" },
          "directives": {
            "design_intent": "string or null",
            "critical_rules": "string or null",
            "variants": ["string"] or null
          }
        }
      ]
    }
    """

def compile_yori_gemini(file_path):
    # 1. Validación básica
    if "TU_API_KEY" in API_KEY:
        print("❌ ERROR: No has configurado tu API KEY en el script.")
        print("   Edita el archivo y pega tu clave de Google AI Studio.")
        return None
        
    if not os.path.exists(file_path):
        print(f"❌ Error: El archivo '{file_path}' no existe.")
        return None

    print(f"🏭 [M-MACHINE] Leyendo: {file_path}")
    with open(file_path, 'r', encoding='utf-8') as f:
        yori_content = f.read()

    print(f"⚡ [Gemini New SDK] Compilando con {MODEL_ID}...")

    try:
        # 2. Inicializar Cliente (Sintaxis Nueva)
        client = genai.Client(api_key=API_KEY)
        
        # 3. Generación con configuración tipada
        response = client.models.generate_content(
            model=MODEL_ID,
            contents=f"SYSTEM INSTRUCTION:\n{get_system_prompt()}\n\nUSER INPUT:\nCompile this Yori code:\n\n{yori_content}",
            config=types.GenerateContentConfig(
                response_mime_type="application/json" 
            )
        )
        
        # En la nueva librería, response.text suele venir limpio
        return response.text

    except Exception as e:
        print(f"❌ Error de API de Google: {e}")
        return None

# --- MAIN ---
if __name__ == "__main__":
    input_file = "app.yori"
    if len(sys.argv) > 1:
        input_file = sys.argv[1]

    json_result = compile_yori_gemini(input_file)
    
    if json_result:
        output_file = input_file.replace(".yori", ".json")
        
        try:
            # Guardar el Blueprint
            with open(output_file, "w", encoding='utf-8') as f:
                f.write(json_result)
            
            print("-" * 40)
            print(f"✅ COMPILACIÓN EXITOSA")
            print(f"📄 Blueprint generado: {output_file}")
            print("-" * 40)
            
            # Validación visual
            try:
                parsed = json.loads(json_result)
                print(json.dumps(parsed, indent=2))
            except:
                print(json_result)
            
        except IOError as e:
            print(f"❌ Error escribiendo archivo: {e}")