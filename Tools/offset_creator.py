import json
import re
import os
import sys

# ===== VERIFICAÇÃO DE ARGUMENTO =====
if len(sys.argv) < 2:
    print("❌ Uso: python gerar_offsets.py /caminho/para/o/diretorio")
    sys.exit(1)

# Diretório passado como argumento
sprite_folder = sys.argv[1]

# Caminhos para os arquivos
texture_json_path = os.path.join(sprite_folder, "texture.json")
output_cpp_path = os.path.join(sprite_folder, "offsets.cpp")

# Verificações
if not os.path.isfile(texture_json_path):
    print(f"❌ Arquivo 'texture.json' não encontrado em: {sprite_folder}")
    sys.exit(1)

# ===== LEITURA DO JSON =====
with open(texture_json_path) as f:
    texture_data = json.load(f)

# ===== FUNÇÃO AUXILIAR =====
def extract_frame_number(filename):
    match = re.search(r'Attack(\d+)\.png', filename)
    if match:
        return int(match.group(1))
    return -1

# ===== GERA O CÓDIGO C++ =====
with open(output_cpp_path, "w") as out_file:
    out_file.write("// Vetor com os offsets de pivô\n")
    out_file.write("std::vector<Vector2> runOffsets = {\n")

    for frame in texture_data["frames"]:
        filename = frame["filename"]
        pivot = frame.get("pivot", {"x": 0.0, "y": 0.0})
        frame_data = frame.get("frame", {"w": 0, "h": 0})
        pivot_x_px = pivot["x"] * frame_data["w"]
        pivot_y_px = pivot["y"] * frame_data["h"]

        out_file.write(f"    Vector2({pivot_x_px:.1f}, {pivot_y_px:.1f}), // {filename}\n")

    out_file.write("};\n\n")

    out_file.write("// Ordem da animação, número do frame extraído do nome do arquivo (zero-based)\n")
    out_file.write("std::vector<int> runAnimationOrder = {\n")

    for frame in texture_data["frames"]:
        filename = frame["filename"]
        number = extract_frame_number(filename)
        if number >= 0:
            number -= 1  # índice zero
        out_file.write(f"    {number}, // {filename}\n")

    out_file.write("};\n")

print(f"✅ Arquivo '{output_cpp_path}' gerado com sucesso!")
