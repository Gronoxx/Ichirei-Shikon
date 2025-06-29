import os
import re
import json
import sys

# ... (função sanitize_name continua a mesma) ...

def sanitize_name(name):
    """Limpa o nome para ser usado como chave no JSON."""
    return re.sub(r'[^a-zA-Z0-9_-]', '', name).lower()
def natural_sort_key(s):
    """Permite ordenação natural (ex: spr_1.png, spr_2.png, ..., spr_10.png)"""
    return [int(text) if text.isdigit() else text.lower() for text in re.split(r'(\d+)', s)]


# --- Lógica Principal ---

if len(sys.argv) < 2:
    print("❌ Uso: python3 json_generator.py /caminho/para/o/diretorio/personagem/")
    sys.exit(1)

base_path = sys.argv[1]
if not os.path.isdir(base_path):
    print("❌ Diretório inválido:", base_path)
    sys.exit(1)

project_root = os.path.abspath(os.path.join(base_path, "..","..", ".."))  # Ajuste conforme necessário


# Começamos com um dicionário vazio
final_json = {
    "animations": {}
}

# ... (toda a lógica de iterar pelas pastas e processar as animações continua exatamente a mesma) ...
subfolders = sorted([
    f for f in os.listdir(base_path)
    if os.path.isdir(os.path.join(base_path, f))
])

for folder in subfolders:
    folder_path = os.path.join(base_path, folder)
    clean_name = sanitize_name(folder)
    uppercase_filename_base = folder.upper()
    texture_filename = f"{uppercase_filename_base}.png"
    json_filename = f"{uppercase_filename_base}.json"

    texture_path_full = os.path.join(folder_path, texture_filename)
    data_json_path_full = os.path.join(folder_path, json_filename)

    # === Caso 1: Sprite Sheet + JSON (modo antigo) ===
    if os.path.exists(texture_path_full) and os.path.exists(data_json_path_full):
        try:
            with open(data_json_path_full, 'r') as f_anim:
                anim_data = json.load(f_anim)
                num_frames = len(anim_data['frames'])
                if num_frames == 0:
                    print(f"⚠️ Nenhum frame definido em '{json_filename}' para '{folder}'")
                    continue
                frame_order = list(range(num_frames))
        except (json.JSONDecodeError, KeyError, TypeError) as e:
            print(f"❌ Erro ao processar '{data_json_path_full}': {e}")
            continue

        print(f"\n📂 Processando (sprite sheet): {folder} ({num_frames} frames)")

        fps = num_frames

        final_json["animations"][clean_name] = {
            "texturePath": os.path.relpath(texture_path_full, project_root),
            "dataPath": os.path.relpath(data_json_path_full, project_root),
            "frameOrder": frame_order,
            "fps": fps
        }

    # === Caso 2: Vários PNGs (modo novo) ===
    else:
        image_files = sorted([
            f for f in os.listdir(folder_path)
            if f.lower().endswith('.png')
        ], key=natural_sort_key)

        if not image_files:
            print(f"⚠️ Pulando '{folder}' (nenhuma imagem .png ou JSON válido encontrado)")
            continue

        print(f"\n📂 Processando (frames soltos): {folder} ({len(image_files)} frames)")

        frame_paths = [os.path.relpath(os.path.join(folder_path, img), project_root) for img in image_files]

        fps = len(image_files)

        final_json["animations"][clean_name] = {
            "frames": frame_paths,
            "fps": fps
        }



# --- NOVO: Definir o Offset de Renderização Global ---
print("\n-----------------------------------------------------")
print("🔧 Definindo o Offset de Renderização Global")
print("Este valor desloca o sprite em relação à posição (x,y) do ator.")
print("Ex: Se o frame tem 96x96 e o pivô é no centro da base, use x=48, y=90.")
print("-----------------------------------------------------")

offset_x, offset_y = 0.0, 0.0
while True:
    try:
        offset_x = float(input("   Qual o offset em X? "))
        offset_y = float(input("   Qual o offset em Y? "))
        break
    except ValueError:
        print("   ❗️ Por favor, insira um número válido.")

# Adiciona o offset ao dicionário principal
final_json["renderOffset"] = {"x": offset_x, "y": offset_y}


# --- Saída Final ---
output_filename = f"{os.path.basename(os.path.normpath(base_path))}.json"
output_path = os.path.join(base_path, output_filename)

with open(output_path, "w") as f:
    json.dump(final_json, f, indent=2)

print(f"\n✅ Manifesto de animação salvo em: {output_path}")