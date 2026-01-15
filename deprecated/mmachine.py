import json
import os
import sys
import subprocess
import time

# --- IMPORTANTE: Usamos la librería nueva ---
try:
    from google import genai
    from google.genai import types
except ImportError:
    print("❌ Error: Falta librería. Ejecuta: pip install google-genai")
    sys.exit(1)

# --- CONFIGURACIÓN DE LA FÁBRICA ---
API_KEY = "AIzaSyCBFQPavgZkZ0c0RDJmgJjv9LLL-1CpZ90"  # <--- PEGA TU CLAVE AQUÍ
MODEL_ID = "gemini-2.5-flash" # Modelo estable y rápido

def load_blueprint(json_path):
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"❌ Error leyendo el plano {json_path}: {e}")
        return None

def determine_language_config(blueprint):
    """
    Detecta si debe usar Python o C++ basado en el stack tecnológico.
    """
    tech_stack = blueprint['meta'].get('tech_stack', [])
    stack_str = " ".join(tech_stack).lower()
    
    if "c++" in stack_str or "cpp" in stack_str:
        return {
            "lang": "C++",
            "ext": ".cpp",
            "compile_cmd": ["g++", "-fsyntax-only"] # Chequeo rápido
        }
    else:
        return {
            "lang": "Python",
            "ext": ".py",
            "compile_cmd": [sys.executable, "-m", "py_compile"]
        }

def generate_code_with_gemini(client, prompt, lang_config, previous_error=None):
    lang = lang_config['lang']
    
    if previous_error:
        print(f"🧬 [EVOLUCIÓN] Mutando código {lang} para corregir error...")
        final_prompt = f"""
        ROLE: Expert {lang} Software Engineer.
        TASK: Fix the code based on this compilation error.
        
        PREVIOUS ERROR:
        {previous_error}
        
        ORIGINAL REQUIREMENT:
        {prompt}
        
        OUTPUT: Return ONLY the corrected {lang} code (no markdown).
        """
    else:
        final_prompt = f"""
        ROLE: Expert {lang} Software Engineer.
        TASK: Write a complete, single-file {lang} application based on this Blueprint.
        
        BLUEPRINT (JSON):
        {prompt}
        
        CRITICAL INSTRUCTIONS:
        1. Use the libraries specified in 'tech_stack'.
        2. Implement ALL 'critical_rules' strictly.
        3. Follow 'design_intent' for logic/style.
        4. If C++, include all necessary headers and use 'int main()'.
        
        OUTPUT: Return ONLY the raw {lang} code (no markdown blocks).
        """

    try:
        response = client.models.generate_content(
            model=MODEL_ID,
            contents=final_prompt
        )
        
        if not response.text:
            print("⚠️ ALERTA: Respuesta vacía de la IA.")
            return None

        # Limpieza de Markdown
        code = response.text
        if "```" in code:
            lines = code.split('\n')
            clean_lines = [l for l in lines if not l.strip().startswith("```")]
            code = "\n".join(clean_lines)
            
        return code.strip()

    except Exception as e:
        print(f"❌ Error crítico en llamada API: {e}")
        return None

def test_compilation(file_path, lang_config):
    """
    Ejecuta el compilador y activa el FRENO DE EMERGENCIA si es necesario.
    """
    cmd = lang_config['compile_cmd'] + [file_path]
    
    try:
        subprocess.check_call(cmd, stderr=subprocess.STDOUT)
        return True, None
        
    except subprocess.CalledProcessError as e:
        # Capturamos el error
        error_output = e.output.decode() if e.output else "Unknown Error"
        
        # --- 🚨 EL FRENO DE EMERGENCIA (Environment Guard) ---
        # Si el error coincide con esta lista, NO ES CULPA DE LA IA.
        # Es culpa del sistema (falta instalar cosas).
        fatal_triggers = [
            "No such file or directory",   # C++: Falta el .h
            "fatal error:",                # C++: Error crítico
            "cannot find -l",              # C++: Falta la librería linkeada
            "command not found",           # Sistema: No existe g++
            "is not recognized",           # Windows: No existe g++
            "ModuleNotFoundError"          # Python: Falta pip install
        ]
        
        for trigger in fatal_triggers:
            if trigger.lower() in error_output.lower():
                return False, f"FATAL_ENV_ERROR: {error_output}"
        
        # Si no es fatal, es un error de sintaxis normal (Culpa de la IA)
        return False, error_output

    except FileNotFoundError:
        return False, f"FATAL_ENV_ERROR: No se encontró el compilador '{cmd[0]}'. Verifica tu PATH."
    except Exception as e:
        return False, str(e)

# --- BUCLE PRINCIPAL DE LA MÁQUINA M ---
def run_factory():
    print("🏭 [M-MACHINE] Iniciando Secuencia de Ensamblaje Continua...")
    
    if "TU_API_KEY" in API_KEY:
        print("❌ ERROR: Configura tu API KEY en el script.")
        return

    blueprint = load_blueprint("app.json")
    if not blueprint: return

    lang_config = determine_language_config(blueprint)
    print(f"🔧 Modo detectado: {lang_config['lang']} ({lang_config['ext']})")

    app_name = blueprint['meta'].get('program_name', 'output_app')
    output_filename = f"{app_name.lower()}{lang_config['ext']}"
    
    client = genai.Client(api_key=API_KEY)

    # --- BUCLE EVOLUTIVO INFINITO (HASTA ÉXITO O ERROR FATAL) ---
    current_error = None
    generation = 0
    
    while True:
        generation += 1
        print(f"\n⚙️ [GEN {generation}] Forjando código...")
        
        # A. Generar
        code = generate_code_with_gemini(client, json.dumps(blueprint, indent=2), lang_config, current_error)
        
        if not code:
            print("❌ La IA dejó de responder. Pausando 5s antes de reintentar...")
            time.sleep(5)
            continue # Reintentamos misma generación

        # B. Materializar
        with open(output_filename, "w", encoding='utf-8') as f:
            f.write(code)
        print(f"   💾 Archivo escrito: {output_filename}")

        # C. Test de Integridad
        print(f"   🛡️ Verificando integridad...")
        success, error_msg = test_compilation(output_filename, lang_config)

        if success:
            print(f"\n✨ [ÉXITO TOTAL] El código es válido en la Generación {generation}.")
            print(f"   (Bucle terminado)")
            if lang_config['lang'] == "C++":
                print(f"🚀 Compilar final: g++ {output_filename} -o {app_name}.exe")
            else:
                print(f"🚀 Ejecutar: python {output_filename}")
            break # ROMPEMOS EL BUCLE POR ÉXITO
        
        else:
            # Analizamos si activar el Freno de Emergencia
            if "FATAL_ENV_ERROR" in error_msg:
                print(f"\n🛑 [FRENO DE EMERGENCIA ACTIVADO]")
                print(f"   La Máquina ha detectado un error de entorno que la IA no puede arreglar.")
                print(f"   ------------------------------------------------------------")
                # Limpiamos el mensaje para que sea legible
                clean_msg = error_msg.replace("FATAL_ENV_ERROR:", "").strip()
                # Mostramos las primeras 3 líneas del error
                print(f"   CAUSA: {' '.join(clean_msg.splitlines()[:3])}...") 
                print(f"   ------------------------------------------------------------")
                print("   >>> ACCIÓN HUMANA REQUERIDA: Instala la librería faltante.")
                break # ROMPEMOS EL BUCLE POR ERROR FATAL
            
            else:
                print(f"   ⚠️ [FALLO EVOLUTIVO] Código roto. Mutando...")
                # Mostramos un fragmento del error para que sepas qué está pasando
                print(f"      Error: {error_msg.splitlines()[0][:100]}...")
                current_error = error_msg
                # El bucle continúa automáticamente (Infinite Loop)

if __name__ == "__main__":
    try:
        run_factory()
    except KeyboardInterrupt:
        print("\n\n🛑 Operación detenida manualmente por el usuario.")